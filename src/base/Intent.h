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

#ifndef BASE_INTENT_H_
#define BASE_INTENT_H_

#include <map>
#include <string>
#include <pbnjson.hpp>

#include "interface/IClassName.h"
#include "interface/ISerializable.h"

using namespace std;
using namespace pbnjson;

class Intent : public IClassName
             , public ISerializable {
public:
    Intent(string category, JValue base = Object());
    virtual ~Intent();

    void setAction(string action) { m_action = action; }
    void setUri(string uri) { m_uri = uri; }
    void setDisplay(JValue display) { m_display = display; }
    void setExtra(JValue extra) { m_extra = extra; }

    JValue& getBase() { return m_base; }
    const string& getCategory() { return m_category; }
    const string& getAction() { return m_action; }
    const string& getUri() { return m_uri; }
    const JValue& getDisplay() { return m_display; }
    const JValue& getExtra() { return m_extra; }

    bool toJson(JValue& json);

private:
    JValue m_base;
    string m_category;
    string m_action;
    string m_uri;
    JValue m_display;
    JValue m_extra;
};

typedef shared_ptr<Intent> IntentPtr;

#endif /* BASE_INTENT_H_ */