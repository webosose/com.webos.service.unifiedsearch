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

#include "DB8Category.h"

DB8Category::DB8Category(const string& id, const string& name, const JValue& kind)
    : Category(id, name)
    , m_kind(kind)
{

}

IntentPtr DB8Category::generateIntent(SearchItemPtr item)
{
    auto intent = make_shared<Intent>(getCategoryName());
    intent->setAction("view");
    intent->setUri(item->getKey());
    intent->setDisplay(item->getDisplay());
    intent->setExtra(item->getExtra());
    return intent;
}