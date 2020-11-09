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

#include "DB8.h"

map<string, shared_ptr<DB8>> DB8::s_instances;

shared_ptr<DB8> DB8::getDB(string service) {
    auto it = s_instances.find(service);
    if (it != s_instances.end()) {
        return it->second;
    } else {
        shared_ptr<DB8> dbPtr;
        dbPtr.reset(new DB8(service));
        dbPtr->initialize();
        s_instances.insert({service, dbPtr});
        return dbPtr;
    }
}

DB8::DB8(string service)
    : LunaClient(service)
    , m_callId(0)
{
}

void DB8::onInitialzed()
{
    // nothing
}

void DB8::onFinalized()
{

}

void DB8::onServerStatusChanged(bool isConnected)
{

}

bool DB8::find(string kind, string key, string value, databaseCB callback)
{
    if (!isConnected()) {
        Logger::warning("DB8", __FUNCTION__, getName() + " is not connected");
        return false;
    }

    static string method = string("luna://") + getName() + string("/find");

    JValue requestPayload = Object();
    requestPayload.put("query", Object());
    requestPayload["query"].put("from", kind);

    JValue where = Object();
    where.put("prop", key);
    where.put("op", "%");
    where.put("val", value);
    where.put("collate", "primary");
    requestPayload["query"].put("where", Array());
    requestPayload["query"]["where"].append(where);

    call(method, requestPayload.stringify(), [this, key, value, callback] (LSMessage *message) -> bool {
        Message response(message);
        JValue responsePayload = JDomParser::fromString(response.getPayload());
        Logger::logCallResponse("DB8", __FUNCTION__, response, responsePayload);

        if (responsePayload.isNull())
            return false;

        bool returnValue = false;
        if (!responsePayload.hasKey("returnValue") || (responsePayload["returnValue"].asBool(returnValue) != CONV_OK) || !returnValue)
            return false;

        JValue results = responsePayload["results"];
        if (!results.isArray())
            return false;

        callback(results);
        return true;
    });

    return true;
}