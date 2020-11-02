// Copyright (c) 2020 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "AppContents.h"

#include "base/Database.h"
#include "base/CategoryList.h"
#include "bus/client/SettingService.h"
#include "util/File.h"
#include "util/JValueUtil.h"

AppContents::AppContents(string name, JValue &app)
    : Category(name)
    , m_appInfo(app)
{
    setClassName("AppContents");

    // FIXME - no need to re-index installted app
    eraseCategory();

    createIndexes();
}

AppContents::~AppContents()
{
}

bool AppContents::createIndexes()
{
    string indexFile, folderPath, icon;
    JValueUtil::getValue(m_appInfo, "folderPath", folderPath);
    JValueUtil::getValue(m_appInfo, "searchIndex", indexFile);
    JValueUtil::getValue(m_appInfo, "icon", icon);

    string indexFilePath = File::join(folderPath, indexFile);
    JValue indexJson = JDomParser::fromFile(indexFilePath.c_str());

    if (!indexJson.isValid()) {
        Logger::warning(getClassName(), __FUNCTION__, Logger::format("Index file is not exist or invalid format : %s", indexFilePath.c_str()));
        return false;
    }

    JValue items;
    if (!JValueUtil::getValue(indexJson, "items", items) || !items.isArray()) {
        Logger::warning(getClassName(), __FUNCTION__, Logger::format("Index file doesn't have items : %s", indexFilePath.c_str()));
        return false;
    }

    string action, uri;
    JValueUtil::getValue(indexJson, "action", action);
    JValueUtil::getValue(indexJson, "uri", uri);

    // get all labels
    map<string, map<string, string>> allLabels = getLabels();

    JValue display = Object();
    display.put("icon", File::join(folderPath, icon));

    // per search item
    for (auto item : items.items()) {
        string path;
        JValue labelKeys, titles, extra;
        string searchValue;
        bool first = true;

        if (!JValueUtil::getValue(item, "path", path)) {
            Logger::warning(getClassName(), __FUNCTION__, Logger::format("Index doesn't have intent or wrong: %s", item.stringify().c_str()));
            continue;
        }
        if (!JValueUtil::getValue(item, "labels", labelKeys) || !labelKeys.isArray()) {
            Logger::warning(getClassName(), __FUNCTION__, Logger::format("Index doesn't have labels or wrong: %s", item.stringify().c_str()));
            continue;
        }

        // collect labels for a item
        for (auto labelKeyObj : labelKeys.items()) {
            string labelKey = labelKeyObj.asString();
            auto it = allLabels.find(labelKey);
            if (it == allLabels.end()) {
                Logger::warning(getClassName(), __FUNCTION__, Logger::format("Resources doesn't have '%s' key.", labelKey.c_str()));
                continue;
            }

            auto labelLangs = it->second;
            // If there is no explicit 'en' value, use labelKey as 'en' value.
            if (labelLangs.find("en") == labelLangs.end()) {
                labelLangs.insert({"en", labelKey});
            }

            // Use first label as "title" of the item
            if (titles.isNull()) {
                titles = Object();
                for (auto label : labelLangs) {
                    titles.put(label.first, label.second);
                }
                display.put("title", titles);
            }

            // for all languages
            for (auto label : labelLangs) {
                if (first) {
                    first = false;
                } else {
                    searchValue += ", ";
                }
                searchValue += label.second;
            }
        }

        // if no searchValue, ignore it
        if (searchValue.size() == 0) {
            Logger::warning(getClassName(), __FUNCTION__, Logger::format("No resources for '%s' path.", path.c_str()));
            continue;
        }

        // get extra for item
        JValueUtil::getValue(item, "extra", extra);

        SearchItemPtr sItem = make_shared<SearchItem>(getCategoryName(), uri + path, searchValue, display, extra);
        Database::getInstance().insert(sItem);
    }

    return true;
}

IntentPtr AppContents::generateIntent(SearchItemPtr item)
{
    auto intent = make_shared<Intent>(getCategoryName());
    intent->setAction("view");
    intent->setUri(item->getKey());
    intent->setExtra(item->getExtra());

    JValue title, display = item->getDisplay().duplicate();
    string curTitle, icon;

    // replace title object to string by choosing language
    if (JValueUtil::getValue(display, "title", title) && title.isObject()) {
        string lang = SettingService::getInstance().language();
        if (!JValueUtil::getValue(title, lang, curTitle)) {
            JValueUtil::getValue(title, "en", curTitle); // fallback = en
        }
    }
    display.put("title", curTitle);
    intent->setDisplay(display);

    return intent;
}

bool AppContents::eraseCategory()
{
    Database::getInstance().remove(getCategoryName());
    return true;
}

/**
 * Get ALL labels from app's resource files
 *
 * return:
 *  - first key = labelKey
 *  - second key = language (e.g. 'en', 'ko')
 *  - third value = label Value
 */
map<string, map<string, string>> AppContents::getLabels()
{
    string folderPath;
    JValueUtil::getValue(m_appInfo, "folderPath", folderPath);

    // open manifest and get label filenames
    static const string manifestFileName = "ilibmanifest.json";
    string resourceFolder = File::join(folderPath, "resources");
    string maniFilePath = File::join(resourceFolder, manifestFileName);

    JValue manifest = JDomParser::fromFile(maniFilePath.c_str());

    map<string, map<string, string>> allLabels;

    JValue labelFiles;
    if (JValueUtil::getValue(manifest, "files", labelFiles) && labelFiles.isArray()) {
        // read each label file (for now, all language will be loaded)
        for (auto labelFileObj : labelFiles.items()) {
            string labelFile = labelFileObj.asString();
            if (labelFile.empty() || labelFile == manifestFileName)
                continue;

            string language = labelFile.substr(0, labelFile.find("/"));
            JValue labels = JDomParser::fromFile(File::join(resourceFolder, labelFile).c_str());
            for (auto label : labels.children()) {
                string key = label.first.asString();
                string value = label.second.asString();
                if (allLabels.find(key) == allLabels.end()) {
                    allLabels.insert({key, map<string, string>()});
                }
                // concat all languages
                allLabels[key][language] = value;
            }
        }
    }

    return allLabels;
}

bool AppContentsList::add(JValue &app)
{
    string searchIndex, type, id, title;

    // only create 'searchIndex' field exist
    if (JValueUtil::getValue(app, "searchIndex", searchIndex) && !searchIndex.empty()) {
        JValueUtil::getValue(app, "type", type);
        JValueUtil::getValue(app, "id", id);
        JValueUtil::getValue(app, "title", title);

        if (type != "web") {
            Logger::warning(getClassName(), __FUNCTION__, Logger::format("Currently, only support 'web' type. %s=%s", id.c_str(), type.c_str()));
            return false;
        }

        Logger::warning(getClassName(), __FUNCTION__, Logger::format("Try to parse %s: %s", id.c_str(), searchIndex.c_str()));
        auto appContent = make_shared<AppContents>(title, app);
        CategoryList::getInstance().addCategory(appContent);
        m_appContentsMap.insert({id, appContent});
        return true;
    }

    return false;
}

bool AppContentsList::remove(string &id)
{
    auto it = m_appContentsMap.find(id);
    if (it == m_appContentsMap.end()) {
        return false;
    }

    it->second->eraseCategory();
    m_appContentsMap.erase(it);
    CategoryList::getInstance().removeCategory(it->second->getCategoryName());
    return true;
}
