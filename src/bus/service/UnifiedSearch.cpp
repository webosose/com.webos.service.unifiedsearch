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

#include "base/SearchManager.h"

#include "util/JValueUtil.h"
#include "util/Time.h"

UnifiedSearch::UnifiedSearch()
    : LS::Handle(LS::registerService("com.webos.service.unifiedsearch"))
{
    setClassName("UnifiedSearch");

    LS_CATEGORY_BEGIN(UnifiedSearch, "/")
        LS_CATEGORY_METHOD(search)
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

    Logger::logAPIRequest(getClassName(), __FUNCTION__, task->request(), requestPayload);

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

UnifiedSearch::LunaResTask::LunaResTask(string className, string funcName, LSMessage *msg)
    : m_className(className)
    , m_funcName(funcName)
    , m_message(msg)
    , m_response(Object())
{
    m_request = JDomParser::fromString(m_message.getPayload());
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