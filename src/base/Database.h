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

#ifndef BASE_DATABASE_H_
#define BASE_DATABASE_H_

#include <map>
#include <string>

#include <sqlite3.h>

#include "Category.h"
#include "DataSource.h"
#include "SearchItem.h"

#include "interface/IInitializable.h"
#include "interface/ISingleton.h"

#define RANK_MAX 999999

using namespace std;

class Database : public DataSource
               , public IInitializable<Database>
               , public ISingleton<Database> {
friend class ISingleton<Database>;
public:
    virtual ~Database();

    bool onInitialization();
    bool onFinalization();

    bool adjustOrCreateCategory(CategoryPtr cate);
    bool removeCategory(string cateId);
    bool updateCategory(CategoryPtr cate);
    vector<CategoryPtr> getCategories();

    bool insertItem(SearchItemPtr item);
    bool removeItem(string category, string key = "");

    bool search(string searchKey, searchCB callback);

private:
    Database();

    bool updateRanks(int value, int start, int end);

    sqlite3* m_database;
    map<string, sqlite3_stmt*> m_statements;
};

#endif /* BASE_DATABASE_H_ */