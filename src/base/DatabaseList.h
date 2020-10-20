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

#ifndef BASE_DATABASELIST_H_
#define BASE_DATABASELIST_H_

#include <map>
#include <vector>
#include <string>
#include <sqlite3.h>
#include <pbnjson.hpp>

#include "base/Database.h"
#include "base/SearchItem.h"

#include "interface/IClassName.h"
#include "interface/ISingleton.h"
#include "interface/IInitializable.h"

using namespace std;

class DatabaseList: public IInitializable,
                public ISingleton<DatabaseList> {
friend class ISingleton<DatabaseList>;
public:
    virtual ~DatabaseList();

    bool onInitialization();
    bool onFinalization();

    bool createDB(string category);
    bool deleteDB(string category);
    DatabasePtr getDB(string category);

    vector<SearchItemPtr> search(string searchKey);

private:
    DatabaseList();

    map<string, DatabasePtr> m_databases;
};

#endif /* BASE_DATABASE_H_ */
