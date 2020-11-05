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
#include "base/CategoryList.h"
#include "util/File.h"

SAM::SAM() : AbsLunaClient("com.webos.applicationManager")
{
    setClassName("SAM");
}

SAM::~SAM()
{
}

void SAM::onInitialzed()
{
    m_contentList = make_shared<AppContentsList>();
    m_applications = make_shared<Applications>();
    CategoryList::getInstance().addCategory(m_applications);
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

        m_listAppsCall = UnifiedSearch::getInstance().callMultiReply(
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
    auto contentList = sam->m_contentList;

    Message response(message);
    JValue subscriptionPayload = JDomParser::fromString(response.getPayload());
    Logger::logSubscriptionResponse(sam->getClassName(), __FUNCTION__, response, subscriptionPayload);

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
            auto appContent = contentList->find(id);
            if (appContent) {
                appContent->setCategoryName(title);
                countUpdate++;
            } else {
                if (contentList->add(lp)) {
                    countAdd++;
                }
            }
        }
        Logger::info(sam->getClassName(), __FUNCTION__, Logger::format("Added: %d, Updated: %d", countAdd, countUpdate));
    } else if (JValueUtil::getValue(subscriptionPayload, "change", change)) {
        // Second~ (changed)
        JValue app = Object();
        if (JValueUtil::getValue(subscriptionPayload, "app", app)) {
            if (change == "added") {
                appInst->addToDatabase(app);
                contentList->add(app);
                Logger::info(sam->getClassName(), __FUNCTION__, "Add a item");
            } else if (change == "removed") {
                string id;
                JValueUtil::getValue(app, "id", id);
                appInst->removeFromDatabase(id);
                contentList->remove(id);
                Logger::info(sam->getClassName(), __FUNCTION__, "Remove a item");
            }
        }
    }

    return true;
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