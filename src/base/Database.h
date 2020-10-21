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

#include "base/SearchItem.h"
#include "interface/IInitializable.h"
#include "interface/ISingleton.h"

using namespace std;

class Database : public IInitializable
               , public ISingleton<Database> {
friend class ISingleton<Database>;
public:
    virtual ~Database() {}

    bool onInitialization();
    bool onFinalization();

    bool insert(SearchItemPtr item);
    bool remove(string category, string key = "");

    vector<SearchItemPtr> search(string searchKey);

private:
    Database();

    sqlite3 *m_database;
    sqlite3_stmt *m_insertStmt;
    sqlite3_stmt *m_selectStmt;
};

#endif /* BASE_DATABASE_H_ */
