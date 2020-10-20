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

#include <string>
#include <vector>

#include "base/CategoryList.h"

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

    Message request(&message);
    pbnjson::JValue responsePayload = pbnjson::Object();
    pbnjson::JValue requestPayload = JDomParser::fromString(request.getPayload());

    Logger::logAPIRequest(getInstance().getClassName(), __FUNCTION__, request, requestPayload);

    string searchKey;
    bool ret = true;

    if (JValueUtil::getValue(requestPayload, "key", searchKey) && !searchKey.empty()) {
        JValue intentArr = Array();
        auto intents = CategoryList::getInstance().search(searchKey);
        for (auto intent : intents) {
            JValue obj;
            intent->toJson(obj);
            intentArr << obj;
        }
        responsePayload.put("intents", intentArr);
    } else {
        responsePayload.put("errorText", "'key' isn't specified.");
        ret = false;
    }

    responsePayload.put("returnValue", ret);
    request.respond(responsePayload.stringify().c_str());
    Logger::logAPIResponse(getInstance().getClassName(), __FUNCTION__, request, responsePayload);
    return ret;
}
