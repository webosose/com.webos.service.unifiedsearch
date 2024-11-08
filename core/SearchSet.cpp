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

#include "SearchSet.h"

#include "Logger.h"

bool SearchSet::addCategory(CategoryPtr category)
{
    if (!category) {
        Logger::warning("SearchSet", __FUNCTION__, "Null category");
        return false;
    }

    string id = category->getCategoryId();
    if (m_categories.find(id) != m_categories.end()) {
        Logger::warning("SearchSet", __FUNCTION__, Logger::format("Already exist category: %s", id.c_str()));
        return false;
    }

    m_categories.insert({id, category});

    if (m_client) {
        m_client->categoryAdded(std::move(category));
    }
    return true;
}

bool SearchSet::removeCategory(const string& id)
{
    if (m_categories.find(id) == m_categories.end()) {
        Logger::warning("SearchSet", __FUNCTION__, Logger::format("Not exist category: %s", id.c_str()));
        return false;
    }

    m_categories.erase(id);

    if (m_client) {
        m_client->categoryRemoved(id);
    }
    return true;
}

CategoryPtr SearchSet::findCategory(const string& id)
{
    auto category = m_categories.find(id);
    if (category != m_categories.end()) {
        return category->second;
    }
    return nullptr;
}