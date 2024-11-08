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

#ifndef BASE_CORE_CATEGORY_H_
#define BASE_CORE_CATEGORY_H_

#include <map>
#include <string>
#include <functional>
#include <pbnjson.hpp>

#include "Intent.h"
#include "SearchItem.h"

using namespace std;
using namespace pbnjson;

class Category {
public:
    Category(const string& id, const string& name = "");
    virtual ~Category();

    void setCategoryName(const string& name) { m_name = name; }
    void setRank(int rank) { m_rank = rank; }
    void setEnabled(bool enabled) { m_enabled = enabled; }

    const string& getCategoryId() { return m_id; }
    const string& getCategoryName() { return m_name; }
    int getRank() { return m_rank; }
    bool isEnabled() { return m_enabled; }

    virtual IntentPtr generateIntent(SearchItemPtr item) { return nullptr; };

protected:
    string m_id;
    string m_name;
    int m_rank;
    bool m_enabled;
};

typedef shared_ptr<Category> CategoryPtr;

#endif /* BASE_CATEGORY_H_ */