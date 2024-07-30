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

#ifndef BUS_CLIENT_DB8_H_
#define BUS_CLIENT_DB8_H_

#include <pbnjson.hpp>
#include <map>

#include "LunaClient.h"
#include "Logger.h"

using namespace std;
using namespace LS;
using namespace pbnjson;


class DB8 : public LunaClient {
public:
    ~DB8() {}

    using databaseCB = function<void(JValue &results)>;
    bool find(string kind, string key, string value, databaseCB callback);

    static shared_ptr<DB8> getDB(const string& service);

protected:
    // LunaClient
    void onInitialzed() override;
    void onFinalized() override;
    void onServerStatusChanged(bool isConnected) override;

private:
    DB8(const string& service);

    int m_callId;
    map<string, Call> m_calls;

    static map<string, shared_ptr<DB8>> s_instances;
};

typedef shared_ptr<DB8> DB8Ptr;

#endif /* BUS_CLIENT_DB8_H_ */
