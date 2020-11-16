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

#include "bus/service/UnifiedSearch.h"
#include "LunaClient.h"

#include <string>
#include <vector>
#include <thread>

#include "base/Database.h"
#include "base/SearchManager.h"

#include "util/JValueUtil.h"
#include "util/Time.h"

UnifiedSearch::UnifiedSearch()
    : LS::Handle(LS::registerService("com.webos.service.unifiedsearch"))
{
    LS_CATEGORY_BEGIN(UnifiedSearch, "/")
        LS_CATEGORY_METHOD(search)
        LS_CATEGORY_METHOD(getCategories)
        LS_CATEGORY_METHOD(setCategoryInfo)
    LS_CATEGORY_END
}

UnifiedSearch::~UnifiedSearch()
{
}

bool UnifiedSearch::onInitialization()
{
    try {
        Handle::attachToLoop(m_mainloop);
        LunaClient::setMainHandle(this);
    } catch(exception& e) {
    }
    return true;
}

bool UnifiedSearch::onFinalization()
{
    return true;
}

bool UnifiedSearch::search(LSMessage &message)
{
    auto task = make_shared<LunaResTask>(getClassName(), __FUNCTION__, &message);
    auto requestPayload = task->requestPayload();
    auto responsePayload = task->responsePayload();

    string key;
    if (!JValueUtil::getValue(requestPayload, "key", key) && key.empty()) {
        responsePayload.put("errorText", "'key' isn't specified.");
        responsePayload.put("returnValue", false);
        return false;
    }

    responsePayload.put("returnValue", true);
    responsePayload.put("intents", Object());

    // search from SearchManager
    auto allIntents = SearchManager::getInstance()->search(key, [this, task] (map<string, vector<IntentPtr>> allIntents) {
        for (auto category : allIntents) {
            auto intents = category.second;
            JValue intentArr = Array();
            for (auto intent : intents) {
                JValue obj;
                intent->toJson(obj);
                intentArr << obj;
            }
            task->responsePayload()["intents"].put(category.first, intentArr);
        }
    });

    return true;
}

bool UnifiedSearch::getCategories(LSMessage &message)
{
    auto task = make_shared<LunaResTask>(getClassName(), __FUNCTION__, &message);
    auto responsePayload = task->responsePayload();
    auto categories = Database::getInstance()->getCategories();
    JValue cateEnabled = Array();
    JValue cateDisabled = Array();
    for (auto category : categories) {
        JValue cateObj = Object();
        cateObj.put("id", category->getCategoryId());
        cateObj.put("name", category->getCategoryName());
        if (category->isEnabled()) {
            cateEnabled.append(cateObj);
        } else {
            cateDisabled.append(cateObj);
        }
    }
    responsePayload.put("enabled", cateEnabled);
    responsePayload.put("disabled", cateDisabled);
    responsePayload.put("returnValue", true);
    return true;
}

bool UnifiedSearch::setCategoryInfo(LSMessage &message)
{
    auto task = make_shared<LunaResTask>(getClassName(), __FUNCTION__, &message);
    auto requestPayload = task->requestPayload();
    auto responsePayload = task->responsePayload();

    string id;
    int rank = RANK_MAX;
    bool enabled = true;
    if (!JValueUtil::getValue(requestPayload, "id", id) || id.empty()) {
        responsePayload.put("errorText", "'id' isn't specified.");
        responsePayload.put("returnValue", false);
        return false;
    }

    bool retEna = JValueUtil::getValue(requestPayload, "enabled", enabled);
    bool retRank = JValueUtil::getValue(requestPayload, "rank", rank);
    if (!retEna && !retRank) {
        responsePayload.put("errorText", "Needs to add at least one of 'rank' or 'enabled'.");
        responsePayload.put("returnValue", false);
        return false;
    } else if (enabled && !retRank) {
        responsePayload.put("errorText", "Needs 'rank' if its enabled.");
        responsePayload.put("returnValue", false);
        return false;
    }

    auto category = make_shared<Category>(id);
    category->setRank(rank);
    category->setEnabled(enabled);
    if (!Database::getInstance()->updateCategory(category)) {
        responsePayload.put("errorText", "Category info is not changed.");
        responsePayload.put("returnValue", false);
        return false;
    }

    // apply enabled (For now, the rank is only for getCategories from sqlite3, directly)
    auto org = SearchManager::getInstance()->findCategory(id);
    org->setEnabled(enabled);

    responsePayload.put("returnValue", true);
    return true;
}

UnifiedSearch::LunaResTask::LunaResTask(string className, string funcName, LSMessage *msg)
    : m_className(className)
    , m_funcName(funcName)
    , m_message(msg)
    , m_response(Object())
{
    m_request = JDomParser::fromString(m_message.getPayload());
    Logger::logAPIRequest(getClassName(), __FUNCTION__, m_message, m_request);
}

UnifiedSearch::LunaResTask::~LunaResTask() {
    respond();
}

void UnifiedSearch::LunaResTask::respond() {
    try {
        m_message.respond(m_response.stringify().c_str());
        Logger::logCallResponse(m_className, m_funcName, m_message, m_response);
    } catch(exception& e) {
        Logger::error(m_className, m_funcName, Logger::format("LunaResTask::respond exception: %s\n", e.what()));
    }
}