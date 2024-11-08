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

#include "Intent.h"

Intent::Intent(const string& category, const JValue& base)
    : m_category(category)
    , m_base(base)
{
}

Intent::~Intent()
{
}

bool Intent::toJson(JValue& json)
{
    JValue obj = Object();
    JValue intent = std::move(m_base.duplicate());
    if (!m_action.empty()) {
        intent.put("action", m_action);
    }
    if (!m_uri.empty()) {
        intent.put("uri", m_uri);
    }
    if (!m_extra.isNull()) {
        intent.put("extra", m_extra);
    }
    obj.put("intent", intent);
    obj.put("display", m_display);
    json = std::move(obj);
    return true;
}
