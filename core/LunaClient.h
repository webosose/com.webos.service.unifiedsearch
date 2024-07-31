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

#ifndef BUS_CLIENT_ABSLUNACLIENT_H_
#define BUS_CLIENT_ABSLUNACLIENT_H_

#include <iostream>
#include <map>

#include <luna-service2/lunaservice.hpp>
#include <pbnjson.hpp>

#define TIMEOUT_MAX 5000

using namespace std;
using namespace LS;
using namespace pbnjson;

class LSErrorSafe: public LSError {
public:
    LSErrorSafe()
    {
        LSErrorInit(this);
    }

    ~LSErrorSafe()
    {
        LSErrorFree(this);
    }
};

class LunaClient {
public:
    static JValue& getEmptyPayload();
    static JValue& getSubscriptionPayload();

    LunaClient(const string& name);
    virtual ~LunaClient();

    virtual void initialize() final;
    virtual void finalize() final;

    const string& getName()
    {
        return m_name;
    }

    bool isConnected()
    {
        return m_isConnected;
    }

    using LunaReqCB = function<bool(LSMessage *message)>;
    using LunaReqTaskID = unsigned long;

    LunaReqTaskID call(string method, string payload, LunaReqCB callback = nullptr, bool subscribe = false);
    bool cancel(LunaReqTaskID id);

    static void setMainHandle(LS::Handle *handle);
    static LS::Handle* getMainHandle() { return s_mainHandle; }

protected:
    virtual void onInitialzed() = 0;
    virtual void onFinalized() = 0;
    virtual void onServerStatusChanged(bool isConnected) = 0;

private:
    static bool _onServerStatus(LSHandle* sh, LSMessage* message, void* context);
    static bool _onResponse(LSHandle* sh, LSMessage* message, void* context);

    static LS::Handle *s_mainHandle;
    static vector<LunaClient*> s_deferred;

    class LunaReqTask {
    public:
        LunaReqTask(LunaClient *c, LunaReqTaskID i, LunaReqCB& cb, bool s) : client(c), id(i), callback(cb), isSubscribe(s) {}
        LunaClient *client;
        LunaReqTaskID id;
        LunaReqCB callback;
        bool isSubscribe;
    };

    string m_name;
    bool m_isConnected;
    Call m_statusCall;

    map<LunaReqTaskID, Call> m_callMap;
    map<LunaReqTaskID, shared_ptr<LunaReqTask>> m_taskMap;
    LunaReqTaskID m_callId;
};

#endif /* BUS_CLIENT_ABSLUNACLIENT_H_ */
