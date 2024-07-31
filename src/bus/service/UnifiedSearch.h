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

#ifndef BUS_SERVICE_UNIFIEDSEARCH_H_
#define BUS_SERVICE_UNIFIEDSEARCH_H_

#include <string>
#include <map>
#include <pbnjson.hpp>
#include <luna-service2/lunaservice.hpp>

#include "interface/IInitializable.h"
#include "interface/ISingleton.h"

using namespace LS;
using namespace pbnjson;

class UnifiedSearch: public LS::Handle,
              public ISingleton<UnifiedSearch>,
              public IInitializable<UnifiedSearch> {
friend class ISingleton<UnifiedSearch>;
public:
    virtual ~UnifiedSearch();

protected:
    // IInitializable
    virtual bool onInitialization();
    virtual bool onFinalization();

private:
    UnifiedSearch();

    class LunaResTask {
    public:
        LunaResTask(const string& className, const string& funcName, LSMessage *msg);
        ~LunaResTask();

        Message &request() { return m_message; }
        JValue &requestPayload() { return m_request; }
        JValue &responsePayload() { return m_response; }

        // generally, called on last callback executed automatically
        void respond();

    private:
        string m_className;
        string m_funcName;
        Message m_message;
        JValue m_request;
        JValue m_response;
    };

    bool search(LSMessage &message);
    bool getCategories(LSMessage &message);
    bool updateCategory(LSMessage &message);
};

#endif
