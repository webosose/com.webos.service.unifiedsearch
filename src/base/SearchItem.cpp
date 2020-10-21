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

#include "base/SearchItem.h"

#include "conf/ConfFile.h"
#include "util/File.h"
#include "util/Logger.h"

SearchItem::SearchItem()
{
    setClassName("SearchItem");
}

SearchItem::SearchItem(string category, string key, string value)
    : m_category(category)
    , m_key(key)
    , m_value(value)
{
    setClassName("SearchItem");
}

SearchItem::SearchItem(string category, string key, string value, JValue extra)
    : m_category(category)
    , m_key(key)
    , m_value(value)
    , m_extra(extra)
{
    setClassName("SearchItem");
}

const string SearchItem::getExtraStr()
{
    if (m_extra.isNull())
        return "";

    return string("(") + m_extra.stringify() + ")";
}
