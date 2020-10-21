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

#include "base/CategoryList.h"
#include "base/Database.h"

#include "conf/ConfFile.h"
#include "util/File.h"
#include "util/Logger.h"

CategoryList::CategoryList()
{
    setClassName("CategoryList");
}

CategoryList::~CategoryList()
{
}

bool CategoryList::onInitialization()
{
    return true;
}

bool CategoryList::onFinalization()
{
    return true;
}

bool CategoryList::addCategory(Category* category)
{
    if (!category) {
        Logger::warning(getClassName(), __FUNCTION__, "Null category");
        return false;
    }

    string name = category->getCategoryName();
    if (m_categories.find(name) != m_categories.end()) {
        Logger::warning(getClassName(), __FUNCTION__, Logger::format("Already exist category: %s", name.c_str()));
        return false;
    }

    m_categories.insert({name, category});
    return true;
}

bool CategoryList::removeCategory(string name)
{
    if (m_categories.find(name) == m_categories.end()) {
        Logger::warning(getClassName(), __FUNCTION__, Logger::format("Not exist category: %s", name.c_str()));
        return false;
    }

    m_categories.erase(name);
    return true;
}

Category* CategoryList::find(string name)
{
    auto category = m_categories.find(name);
    if (category != m_categories.end()) {
        return category->second;
    }
    return nullptr;
}

map<string, vector<IntentPtr>> CategoryList::search(string searchKey)
{
    map<string, vector<IntentPtr>> allIntents;

    auto items = Database::getInstance().search(searchKey);
    for (auto c : items) {
        const string &cateName = c->getCategory();
        Category* category = find(cateName);
        if (category) {
            if (allIntents.find(cateName) == allIntents.end()) {
                allIntents.insert({cateName, vector<IntentPtr>()});
            }

            IntentPtr intent = category->generateIntent(c);
            allIntents[cateName].push_back(intent);
            Logger::debug(getClassName(), __FUNCTION__, Logger::format("Item: %s, %s", cateName.c_str(), c->getKey().c_str()));
        } else {
            Logger::warning(getClassName(), __FUNCTION__, Logger::format("Ignore '%s': There is no %s category.",
                c->getKey().c_str(),
                cateName.c_str()));
        }
    }

    Logger::info(getClassName(), __FUNCTION__, Logger::format("Find '%s' => %d item(s)", searchKey.c_str(), items.size()));
    return allIntents;
}
