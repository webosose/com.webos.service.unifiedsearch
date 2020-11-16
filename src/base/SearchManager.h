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

#ifndef BASE_SEARCHMANAGER_H_
#define BASE_SEARCHMANAGER_H_

#include <map>
#include <vector>
#include <string>
#include <sqlite3.h>
#include <pbnjson.hpp>

#include "SearchSet.h"
#include "Intent.h"
#include "SearchItem.h"

#include "interface/IClassName.h"
#include "interface/ISingleton.h"
#include "interface/IInitializable.h"

using namespace std;

class SearchManager : public IInitializable<SearchManager>
                    , public ISingleton<SearchManager>
                    , public SearchSetClient {
friend class ISingleton<SearchManager>;
public:
    virtual ~SearchManager() {}

    bool onInitialization();
    bool onFinalization();

    bool addSearchSet(SearchSetPtr category);
    bool removeSearchSet(string id);
    SearchSetPtr findSearchSet(string id);
    CategoryPtr findCategory(string id);

    using resultCB = function<void(map<string, vector<IntentPtr>>)>;
    bool search(string searchKey, resultCB cb);

    void categoryAdded(CategoryPtr category) override;
    void categoryRemoved(string cateId) override;

private:
    SearchManager() {}

    void loadPlugins();

    class SearchTask {
    public:
        SearchTask(string key, resultCB cb);
        ~SearchTask();

        vector<IntentPtr>& get(const string &category);

    private:
        string m_key;
        resultCB m_callback;
        map<string, vector<IntentPtr>> m_intents;
    };

    map<string, SearchSetPtr> m_searchSets;
};

#endif /* BASE_DATABASE_H_ */
