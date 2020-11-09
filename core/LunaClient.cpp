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

#include "LunaClient.h"
#include "Logger.h"

LS::Handle *LunaClient::s_mainHandle = nullptr;
vector<LunaClient*> LunaClient::s_deferred;

JValue& LunaClient::getEmptyPayload()
{
    static JValue empty;

    if (empty.isNull()) {
        empty = pbnjson::Object();
    }
    return empty;
}

JValue& LunaClient::getSubscriptionPayload()
{
    static JValue subscription;
    if (subscription.isNull()) {
        subscription = pbnjson::Object();
        subscription.put("subscribe", true);
    }
    return subscription;
}

bool LunaClient::_onServerStatus(LSHandle* sh, LSMessage* message, void* context)
{
    LunaClient* client = static_cast<LunaClient*>(context);

    Message response(message);
    JValue payload = JDomParser::fromString(response.getPayload());

    bool connected = false;
    if (payload.isNull() || !payload.hasKey("connected") || (payload["connected"].asBool(connected) != CONV_OK))
        return true;

    ;
    if (connected)
        Logger::info("LunaClient", __FUNCTION__, Logger::format("Service is up: %s", client->getName().c_str()));
    else
        Logger::info("LunaClient", __FUNCTION__, Logger::format("Service is down: %s", client->getName().c_str()));

    client->m_isConnected = connected;
    client->m_callMap.clear();
    client->m_taskMap.clear();
    client->m_callId = 0;
    client->onServerStatusChanged(connected);
    return true;
}

LunaClient::LunaClient(const string& name)
    : m_name(name)
    , m_isConnected(false)
    , m_callId(0)
{
}

LunaClient::~LunaClient()
{
}

void LunaClient::initialize()
{
    if (!s_mainHandle) {
        Logger::info("LunaClient", __FUNCTION__, Logger::format("Main handle isn't ready, defer initialization: %s", getName().c_str()));
        s_deferred.push_back(this);
        return;
    }

    JValue requestPayload = pbnjson::Object();
    requestPayload.put("serviceName", getName());
    m_statusCall = s_mainHandle->callMultiReply(
        "luna://com.webos.service.bus/signal/registerServerStatus",
        requestPayload.stringify().c_str(),
        _onServerStatus,
        this
    );

    onInitialzed();
}

void LunaClient::finalize()
{
    m_statusCall.cancel();
    onFinalized();
}

LunaClient::LunaReqTaskID LunaClient::call(string method, string payload, LunaReqCB callback, bool subscribe)
{
    if (!isConnected()) {
        Logger::warning("LunaClient", __FUNCTION__, Logger::format("[LunaClient::call] '%s' called before connecting.", method.c_str()));
        return 0;
    }

    LunaReqTaskID id = ++m_callId;
    auto task = make_shared<LunaReqTask>(this, id, callback, subscribe);
    if (subscribe) {
        m_callMap.insert({id, getMainHandle()->callMultiReply(method.c_str(), payload.c_str(), _onResponse, task.get())});
        m_taskMap.insert({id, task});
    } else {
        m_callMap.insert({id, getMainHandle()->callOneReply(method.c_str(), payload.c_str(), _onResponse, task.get())});
        m_taskMap.insert({id, task});
    }
    return id;
}

bool LunaClient::cancel(LunaReqTaskID id)
{
    auto it = m_callMap.find(id);
    if (it == m_callMap.end()) {
        return false;
    }
    it->second.cancel();
    m_callMap.erase(id);
    m_taskMap.erase(id);
    return true;
}

bool LunaClient::_onResponse(LSHandle* sh, LSMessage* message, void* context)
{
    LunaReqTask *task = static_cast<LunaReqTask*>(context);
    LunaClient *client = task->client;

    bool ret = true;
    if (task->callback) {
        ret = task->callback(message);
    }

    if (!task->isSubscribe) {
        client->m_callMap.erase(task->id);
        client->m_taskMap.erase(task->id);
    }

    return ret;
}

void LunaClient::setMainHandle(LS::Handle *handle)
{
    if (s_mainHandle) {
        Logger::warning("LunaClient", __FUNCTION__, "[Warning] main handle already exist. Ignored!");
        return;
    }

    s_mainHandle = handle;
    while (s_deferred.size()) {
        auto client = s_deferred.back();
        Logger::info("LunaClient", __FUNCTION__, Logger::format("Main handle is ready now, re-initialization: %s", client->getName().c_str()));
        client->initialize();
        s_deferred.pop_back();
    }
}