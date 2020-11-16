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

#ifndef BASE_CORE_SEARCHSET_H_
#define BASE_CORE_SEARCHSET_H_

#include <map>
#include <string>

#include "DataSource.h"
#include "Category.h"

using namespace std;

class SearchSetClient {
public:
    virtual void categoryAdded(CategoryPtr category) = 0;
    virtual void categoryRemoved(string cateId) = 0;
};

class SearchSet {
public:
    SearchSet(string id, DataSourcePtr source) : m_id(id), m_source(source), m_client(nullptr) {}

    const string& getId() { return m_id; }
    DataSourcePtr getDataSource() { return m_source; }
    map<string, CategoryPtr> getCategories() { return m_categories; }

    bool addCategory(CategoryPtr category);
    bool removeCategory(string id);
    CategoryPtr findCategory(string id);

    void setClient(SearchSetClient* client) { m_client = client; }

private:
    string m_id;
    DataSourcePtr m_source;
    map<string, CategoryPtr> m_categories;

    SearchSetClient* m_client;
};

typedef shared_ptr<SearchSet> SearchSetPtr;

#endif /* BASE_SEARCHSET_H_ */