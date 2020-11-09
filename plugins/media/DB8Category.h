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

#ifndef PLUGINS_AUDIOCATEGORY_H_
#define PLUGINS_AUDIOCATEGORY_H_

#include <map>
#include <string>
#include <pbnjson.hpp>

#include "Category.h"
#include "DataSource.h"

#include "DB8.h"
#include "Logger.h"

using namespace std;
using namespace pbnjson;

class DB8Category : public Category {
public:
    DB8Category(string id, string name, JValue kind);
    ~DB8Category() {}

    JValue getKind() { return m_kind; }

    IntentPtr generateIntent(SearchItemPtr item) override;

private:
    JValue m_kind;
};

#endif /* PLUGINS_AUDIOCATEGORY_H_ */