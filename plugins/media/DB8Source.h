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

#ifndef PLUGINS_DB8SOURCE_H_
#define PLUGINS_DB8SOURCE_H_

#include <map>
#include <string>
#include <pbnjson.hpp>

#include "Category.h"
#include "DataSource.h"

#include "DB8.h"
#include "Logger.h"

using namespace std;
using namespace pbnjson;

class DB8Source : public DataSource {
public:
    DB8Source(string id, string db);
    ~DB8Source() {}

    bool search(string searchKey, searchCB cb) override;
    bool addKind(string id, JValue kind);
    bool removeKind(string id);

private:
    class SearchTask {
    public:
        SearchTask(string id, string key, searchCB cb);
        ~SearchTask();

        vector<SearchItemPtr>& items() { return m_items; };

    private:
        string m_id;
        string m_key;
        searchCB m_callback;
        vector<SearchItemPtr> m_items;
    };

    DB8Ptr m_db;
    map<string, JValue> m_kindMap;
};

typedef shared_ptr<DB8Source> DB8SourcePtr;

#endif /* PLUGINS_DB8SOURCE_H_ */