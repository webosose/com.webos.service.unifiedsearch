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

#include "DB8Source.h"

DB8Source::DB8Source(string id, string db)
    : DataSource(id)
    , m_db(DB8::getDB(db))
{
}

bool DB8Source::addKind(string id, JValue kind)
{
    if (m_kindMap.find(id) != m_kindMap.end()) {
        Logger::warning("DB8Source", __FUNCTION__, Logger::format("Kind id '%s' already exit", id.c_str()));
        return false;
    }

    if (!kind.isValid() || !kind.hasKey("kind") || !kind.hasKey("targets")) {
        Logger::warning("DB8Source", __FUNCTION__, Logger::format("Invalid kind: %s", kind.stringify().c_str()));
        return false;
    }

    m_kindMap.insert({id, kind});
    return true;
}

bool DB8Source::removeKind(string id)
{
    if (m_kindMap.find(id) == m_kindMap.end()) {
        Logger::warning("DB8Source", __FUNCTION__, Logger::format("Kind id '%s' doesn't exit", id.c_str()));
        return false;
    }

    m_kindMap.erase(id);
    return true;
}

bool DB8Source::search(string searchKey, searchCB cb)
{
    shared_ptr<SearchTask> task = make_shared<SearchTask>(getId(), searchKey, cb);

    // for all registered kind
    for (auto it : m_kindMap) {
        string category = it.first;
        JValue kind = it.second;

        string kindStr = kind["kind"].asString();
        JValue targetObjs = kind["targets"];
        JValue extraWants = kind["extras"];

        // for all targets (DB8 'prop')
        for (auto targetObj : targetObjs.items()) {
            string target = targetObj.asString();
            m_db->find(kindStr, target, searchKey, [this, category, searchKey, target, task, extraWants] (JValue &results) {
                // get item vector from shared task
                vector<SearchItemPtr> &items = task->items();
                // convert DB find result to searchitem & push it
                for (auto result : results.items()) {
                    string file = result["file_path"].asString();
                    string title = result["title"].asString();

                    JValue display = Object();
                    display.put("title", title);
                    if (result.hasKey("artist")) {
                        display.put("artist", result["artist"]);
                    }
                    if (result.hasKey("thumbnail")) {
                        display.put("icon", result["thumbnail"]);
                    }

                    if (extraWants.isArray() && extraWants.arraySize() > 0) {
                        JValue extra = Object();
                        for (auto wantObj : extraWants.items()) {
                            string want = wantObj.asString();
                            extra.put(want, result[want]);
                        }
                        items.push_back(make_shared<SearchItem>(category, file, title, display, extra));
                    } else {
                        items.push_back(make_shared<SearchItem>(category, file, title, display));
                    }
                }
                Logger::info("DB8Source", "search", Logger::format("Find '%s' => %d item(s) on %s:%s", searchKey.c_str(), results.arraySize(), category.c_str(), target.c_str()));
            });
        }
    }
    return true;
}

DB8Source::SearchTask::SearchTask(string id, string key, searchCB cb)
    : m_id(id)
    , m_key(key)
    , m_callback(cb)
{
    Logger::debug("DB8Source", __FUNCTION__, Logger::format("Search task started: %s", key.c_str()));
}

DB8Source::SearchTask::~SearchTask()
{
    if (m_callback) {
        m_callback(m_id, m_items);
    }
    Logger::debug("DB8Source", __FUNCTION__, "Search task ended");
}