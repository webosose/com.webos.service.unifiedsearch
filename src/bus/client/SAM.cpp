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

#include "SAM.h"

#include "base/Database.h"
#include "base/SearchManager.h"
#include "util/File.h"

SAM::SAM() : LunaClient("com.webos.applicationManager")
{
}

SAM::~SAM()
{
}

void SAM::onInitialzed()
{
    m_applications = make_shared<Applications>();
    m_searchSet = make_shared<SearchSet>("SAM", Database::getInstance());
    m_searchSet->addCategory(m_applications);
    SearchManager::getInstance()->addSearchSet(m_searchSet);
}

void SAM::onFinalized()
{
    m_listAppsCall.cancel();
}

void SAM::onServerStatusChanged(bool isConnected)
{
    static string method = string("luna://") + getName() + string("/listApps");

    if (isConnected) {
        JValue requestPayload = pbnjson::Object();
        requestPayload.put("subscribe", true);

        m_listAppsCall = getMainHandle()->callMultiReply(
            method.c_str(),
            requestPayload.stringify().c_str(),
            onListApps,
            this
        );
    } else {
        m_listAppsCall.cancel();
    }
}

bool SAM::onListApps(LSHandle* sh, LSMessage* message, void* context)
{
    auto sam = static_cast<SAM*>(context);
    auto appInst = sam->m_applications;

    Message response(message);
    JValue subscriptionPayload = JDomParser::fromString(response.getPayload());
    Logger::logSubscriptionResponse("SAM", __FUNCTION__, response, subscriptionPayload);

    if (subscriptionPayload.isNull()) {
        return false;
    }

    JValue apps = Object();
    string change;
    if (JValueUtil::getValue(subscriptionPayload, "apps", apps) && apps.isArray()) {
        // when app list comes, remove first
        appInst->removeFromDatabase();

        int countAdd = 0, countUpdate = 0;
        for (auto lp : apps.items()) {
            // add applications
            if (appInst->addToDatabase(lp)) {
                countAdd++;
            }

            // create or update appContent (don't recreate because it's to heavy)
            string id, title;
            JValueUtil::getValue(lp, "id", id);
            JValueUtil::getValue(lp, "title", title);
            auto category = sam->m_searchSet->findCategory(id);
            if (category) {
                category->setCategoryName(title);
                countUpdate++;
            } else {
                if (sam->addAppContents(lp)) {
                    countAdd++;
                }
            }
        }
        Logger::info("SAM", __FUNCTION__, Logger::format("Added: %d, Updated: %d", countAdd, countUpdate));
    } else if (JValueUtil::getValue(subscriptionPayload, "change", change)) {
        // Second~ (changed)
        JValue app = Object();
        if (JValueUtil::getValue(subscriptionPayload, "app", app)) {
            if (change == "added") {
                appInst->addToDatabase(app);
                sam->addAppContents(app);
                Logger::info("SAM", __FUNCTION__, "Add a item");
            } else if (change == "removed") {
                string id;
                JValueUtil::getValue(app, "id", id);
                appInst->removeFromDatabase(id);
                sam->m_searchSet->removeCategory(id);
                Logger::info("SAM", __FUNCTION__, "Remove a item");
            }
        }
    }

    return true;
}

bool SAM::addAppContents(JValue &app)
{
    string searchIndex, type, id, title;

    // only create 'searchIndex' field exist
    if (JValueUtil::getValue(app, "searchIndex", searchIndex) && !searchIndex.empty()) {
        JValueUtil::getValue(app, "type", type);
        JValueUtil::getValue(app, "id", id);
        JValueUtil::getValue(app, "title", title);

        if (type != "web") {
            Logger::warning("SAM", __FUNCTION__, Logger::format("Currently, only support 'web' type. %s=%s", id.c_str(), type.c_str()));
            return false;
        }

        auto appContent = make_shared<AppContents>(id, title, app);
        m_searchSet->addCategory(appContent);
        return true;
    }

    return false;
}

bool SAM::reloadAppsByLocaleChange()
{
    if (!isConnected()) {
        return false;
    }

    // resubscribe
    m_listAppsCall.cancel();
    onServerStatusChanged(true);
    return true;
}