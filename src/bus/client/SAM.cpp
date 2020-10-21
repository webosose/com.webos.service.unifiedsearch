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
    m_listLaunchPointsCall.cancel();
}

void SAM::onServerStatusChanged(bool isConnected)
{
    static string method = string("luna://") + getName() + string("/listLaunchPoints");

    if (isConnected) {
        JValue requestPayload = pbnjson::Object();
        requestPayload.put("subscribe", true);

        m_listLaunchPointsCall = UnifiedSearch::getInstance().callMultiReply(
            method.c_str(),
            requestPayload.stringify().c_str(),
            onListLaunchPoints,
            this
        );
    } else {
        m_listLaunchPointsCall.cancel();
    }
}

bool SAM::addToDatabase(JValue &launchPoint)
{
    string id, title, icon;
    JValueUtil::getValue(launchPoint, "launchPointId", id);
    JValueUtil::getValue(launchPoint, "title", title);
    JValueUtil::getValue(launchPoint, "icon", icon);

    JValue extra = Object();
    extra.put("title", title);
    if (!icon.empty()) {
        extra.put("icon", icon);
    }

    SearchItemPtr item = make_shared<SearchItem>(getCategoryName(), id, title, extra);
    Database::getInstance().insert(item);
    return true;
}

bool SAM::onListLaunchPoints(LSHandle* sh, LSMessage* message, void* context)
{
    SAM *sam = static_cast<SAM*>(context);

    Message response(message);
    JValue subscriptionPayload = JDomParser::fromString(response.getPayload());
    Logger::logSubscriptionResponse(sam->getClassName(), __FUNCTION__, response, subscriptionPayload);

    if (subscriptionPayload.isNull()) {
        return false;
    }

    JValue launchPoints = Object();
    string change;
    if (JValueUtil::getValue(subscriptionPayload, "launchPoints", launchPoints)) {
        // First time, get lists
        if (launchPoints.isArray()) {
            // remove old items first
            // TODO: it's better to update(replace) only updated one - needs better logic
            Database::getInstance().remove(sam->getCategoryName());
            // add new items
            for (auto lp : launchPoints.items()) {
                sam->addToDatabase(lp);
            }
            Logger::info(sam->getClassName(), __FUNCTION__, Logger::format("Add %d item(s)", launchPoints.arraySize()));
        }
    } else if (JValueUtil::getValue(subscriptionPayload, "change", change)) {
        // Second~ (changed)
        JValue launchPoint = Object();
        if (JValueUtil::getValue(subscriptionPayload, "launchPoint", launchPoint)) {
            if (change == "added") {
                sam->addToDatabase(launchPoint);
                Logger::info(sam->getClassName(), __FUNCTION__, "Add a item");
            } else if (change == "removed") {
                string id;
                JValueUtil::getValue(launchPoint, "launchPointId", id);
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

    // FIXME now, add "name" to start intent as 'explicit'
    const static string default_suffix = "_default";
    string key = item->getKey();
    auto pos = key.find(default_suffix);
    if (pos != string::npos) {
        intent->getBase().put("name", key.replace(pos, default_suffix.size(), ""));
    }

    intent->setAction("launch");
    intent->setUri(string("app://") + item->getKey());
    intent->setExtra(item->getExtra());

    return intent;
}
