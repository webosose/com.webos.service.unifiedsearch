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
        LS_CATEGORY_METHOD(updateCategory)
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
    if (!JValueUtil::getValue(requestPayload, "key", key)) {
        responsePayload.put("errorCode", 101);
        responsePayload.put("errorText", "The 'key' isn't specified.");
        responsePayload.put("returnValue", false);
        return false;
    }

    if (key.empty() || key.size() < 2) {
        responsePayload.put("errorCode", 102);
        responsePayload.put("errorText", "The 'key' is empty or too short. (needs >= 2 bytes)");
        responsePayload.put("returnValue", false);
        return false;
    }

    // add results array
    responsePayload.put("results", Array());

    // search from SearchManager
    auto allIntents = SearchManager::getInstance()->search(key, [this, task] (map<string, vector<IntentPtr>> allIntents) {
        // to append results with category ranking
        auto categories = Database::getInstance()->getCategories();
        for (auto category : categories) {
            if (category->isEnabled()) {
                auto cateId = category->getCategoryId();
                auto it = allIntents.find(cateId);
                // if no item allIntents doesn't have it's category. just continue
                if (it == allIntents.end()) {
                    continue;
                }
                auto intents = it->second;

                // create json array
                JValue intentArr = Array();
                for (auto intent : intents) {
                    JValue obj;
                    intent->toJson(obj);
                    intentArr.append(obj);
                }

                // create object and append
                JValue cateObj = Object();
                cateObj.put("categoryId", cateId);
                cateObj.put("items", intentArr);
                task->responsePayload()["results"].append(cateObj);
            }
        }
    });

    responsePayload.put("returnValue", true);
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

bool UnifiedSearch::updateCategory(LSMessage &message)
{
    auto task = make_shared<LunaResTask>(getClassName(), __FUNCTION__, &message);
    auto requestPayload = task->requestPayload();
    auto responsePayload = task->responsePayload();

    string id, name;
    int rank = RANK_MAX;
    bool enabled = true;
    if (!JValueUtil::getValue(requestPayload, "id", id) || id.empty()) {
        responsePayload.put("errorCode", 201);
        responsePayload.put("errorText", "The 'id' isn't specified.");
        responsePayload.put("returnValue", false);
        return false;
    }

    bool retEna = JValueUtil::getValue(requestPayload, "enabled", enabled);
    bool retRank = JValueUtil::getValue(requestPayload, "rank", rank);
    bool retName = JValueUtil::getValue(requestPayload, "name", name);
    if (!retEna && !retRank) {
        responsePayload.put("errorCode", 202);
        responsePayload.put("errorText", "Needs to add at least one of 'rank' or 'enabled'.");
        responsePayload.put("returnValue", false);
        return false;
    } else if (enabled && !retRank) {
        responsePayload.put("errorCode", 203);
        responsePayload.put("errorText", "Needs 'rank' if its enabled.");
        responsePayload.put("returnValue", false);
        return false;
    } else if (!enabled && retRank) {
        responsePayload.put("errorCode", 204);
        responsePayload.put("errorText", "Shouldn't insert 'rank' when its disabled.");
        responsePayload.put("returnValue", false);
        return false;
    }

    auto category = make_shared<Category>(id, retName ? name : "");
    category->setRank(rank);
    category->setEnabled(enabled);
    if (!Database::getInstance()->updateCategory(category)) {
        responsePayload.put("errorCode", 205);
        responsePayload.put("errorText", "Internal database error.");
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