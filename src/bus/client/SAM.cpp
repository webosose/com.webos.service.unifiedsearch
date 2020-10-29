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

SAM::SAM()
    : AbsLunaClient("com.webos.applicationManager")
    , Category("SAM")
{
    setClassName("SAM");
}

SAM::~SAM()
{
}

void SAM::onInitialzed()
{
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

bool SAM::addToDatabase(JValue &app)
{
    bool visible;
    if (JValueUtil::getValue(app, "visible", visible) && !visible) {
        // ignore non-visible apps
        return false;
    }

    string id, title, icon, folderPath;
    JValueUtil::getValue(app, "id", id);
    JValueUtil::getValue(app, "title", title);
    JValueUtil::getValue(app, "folderPath", folderPath);
    JValueUtil::getValue(app, "icon", icon);

    JValue extra = Object();
    extra.put("title", title);
    extra.put("icon", File::join(folderPath, icon));

    // create search item and insert
    SearchItemPtr item = make_shared<SearchItem>(getCategoryName(), id, title, extra);
    return Database::getInstance().insert(item);
}

bool SAM::onListApps(LSHandle* sh, LSMessage* message, void* context)
{
    SAM *sam = static_cast<SAM*>(context);

    Message response(message);
    JValue subscriptionPayload = JDomParser::fromString(response.getPayload());
    Logger::logSubscriptionResponse(sam->getClassName(), __FUNCTION__, response, subscriptionPayload);

    if (subscriptionPayload.isNull()) {
        return false;
    }

    JValue apps = Object();
    string change;
    if (JValueUtil::getValue(subscriptionPayload, "apps", apps)) {
        // First time, get lists
        if (apps.isArray()) {
            // remove old items first
            // TODO: it's better to update(replace) only updated one - needs better logic
            Database::getInstance().remove(sam->getCategoryName());
            // add new items
            for (auto lp : apps.items()) {
                sam->addToDatabase(lp);
            }
            Logger::info(sam->getClassName(), __FUNCTION__, Logger::format("Add %d item(s)", apps.arraySize()));
        }
    } else if (JValueUtil::getValue(subscriptionPayload, "change", change)) {
        // Second~ (changed)
        JValue app = Object();
        if (JValueUtil::getValue(subscriptionPayload, "app", app)) {
            if (change == "added") {
                sam->addToDatabase(app);
                Logger::info(sam->getClassName(), __FUNCTION__, "Add a item");
            } else if (change == "removed") {
                string id;
                JValueUtil::getValue(app, "appId", id);
                Database::getInstance().remove(sam->getCategoryName(), id);
                Logger::info(sam->getClassName(), __FUNCTION__, "Remove a item");
            }
        }
    }

    return true;
}

IntentPtr SAM::generateIntent(SearchItemPtr item)
{
    auto intent = make_shared<Intent>(getCategoryName());
    
/*
    // For now, we using explicit intent because intent manager doesn't accept service as a intent handler.
    intent->setAction("launch");
    intent->setUri(string("app://") + item->getKey());
*/

    // to create explicit intent
    intent->getBase().put("name", item->getKey());

    // set intent
    intent->setExtra(item->getExtra());

    return intent;
}
