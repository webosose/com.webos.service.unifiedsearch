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

#include <dlfcn.h>

#include "Plugin.h"`

#include "base/SearchManager.h"
#include "base/Database.h"

#include "conf/ConfFile.h"
#include "util/File.h"
#include "Logger.h"

SearchManager::SearchManager()
{
    setClassName("SearchManager");
}

SearchManager::~SearchManager()
{
}

bool SearchManager::onInitialization()
{
    loadPlugins();
    return true;
}

bool SearchManager::onFinalization()
{
    return true;
}

bool SearchManager::addSearchSet(SearchSetPtr searchSet)
{
    if (!searchSet) {
        Logger::warning(getClassName(), __FUNCTION__, "Null searchSet");
        return false;
    }

    string id = searchSet->getId();
    if (m_searchSets.find(id) != m_searchSets.end()) {
        Logger::warning(getClassName(), __FUNCTION__, Logger::format("Already exist searchSet: %s", id.c_str()));
        return false;
    }

    m_searchSets.insert({id, searchSet});
    Logger::info(getClassName(), __FUNCTION__, Logger::format("SearchSet added: %s", id.c_str()));
    return true;
}

bool SearchManager::removeSearchSet(string id)
{
    if (m_searchSets.find(id) == m_searchSets.end()) {
        Logger::warning(getClassName(), __FUNCTION__, Logger::format("Not exist searchSet: %s", id.c_str()));
        return false;
    }

    m_searchSets.erase(id);
    Logger::info(getClassName(), __FUNCTION__, Logger::format("SearchSet removed: %s", id.c_str()));
    return true;
}

SearchSetPtr SearchManager::findSearchSet(string id)
{
    auto it = m_searchSets.find(id);
    if (it != m_searchSets.end()) {
        return it->second;
    }
    return nullptr;
}

bool SearchManager::search(string searchKey, resultCB callback)
{
    shared_ptr<SearchTask> task = make_shared<SearchTask>(searchKey, callback);

    // from each source
    for (auto it : m_searchSets) {
        auto id = it.first;
        auto searchSet = it.second;
        auto source = searchSet->getDataSource();
        source->search(searchKey, [this, task, searchSet] (string sourceId, vector<SearchItemPtr> items) {
            // for each items
            for (auto item : items) {
                const string &cateId = item->getCategory();
                CategoryPtr category = searchSet->findCategory(cateId);
                if (!category) {
                    Logger::warning(getClassName(), __FUNCTION__, Logger::format("Ignore '%s': There is no %s category.",
                        item->getKey().c_str(),
                        cateId.c_str()));
                }

                // convert to intent and add to list
                auto cateName = category->getCategoryName();
                auto intent = category->generateIntent(item);
                task->get(cateName).push_back(intent);

                Logger::debug(getClassName(), __FUNCTION__, Logger::format("Item: %s, %s", cateName.c_str(), item->getKey().c_str()));
            }
        });
    }

    return true;
}

SearchManager::SearchTask::SearchTask(string key, resultCB cb)
    : m_key(key)
    , m_callback(cb)
{
    Logger::debug("SearchManager", __FUNCTION__, Logger::format("Search task started: %s", key.c_str()));
}

SearchManager::SearchTask::~SearchTask()
{
    if (m_callback) {
        m_callback(m_intents);
    }
    Logger::debug("SearchManager", __FUNCTION__, "Search task ended");
}

vector<IntentPtr>& SearchManager::SearchTask::get(string &category)
{
    if (m_intents.find(category) == m_intents.end()) {
        m_intents.insert({category, vector<IntentPtr>()});
    }

    return m_intents[category];
}

void SearchManager::loadPlugins()
{
    string plugin_basedir = "/usr/lib/plugins/unifiedsearch";
    auto files = File::readDirectory(plugin_basedir, ".so");
    char *error;
    for (auto file : files) {
        void *handle = dlopen(File::join(plugin_basedir, file).c_str(), RTLD_LAZY);
        if (!handle) {
            Logger::warning(getClassName(), __FUNCTION__, Logger::format("Failed to load: %s", file.c_str()));
            continue;
        }

        plugin_init* create_plugin = (plugin_init*) dlsym(handle, "create_plugin");
        if ((error = dlerror()) != NULL)
        {
            Logger::warning(getClassName(), __FUNCTION__, Logger::format("Failed to load init function: %s", error));
            continue;
        }

        Plugin *plugin = create_plugin();
        addSearchSet(plugin->getSearchSet());

        Logger::info(getClassName(), __FUNCTION__, Logger::format("Plugin loaded: %s", file.c_str()));
    }
}