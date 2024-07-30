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

bool SearchManager::onInitialization()
{
    loadPlugins();
    return true;
}

bool SearchManager::onFinalization()
{
    for (auto handle : m_pluginHandles) {
        dlclose(handle);
    }
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

    searchSet->setClient(this);
    m_searchSets.insert({id, searchSet});
    Logger::info(getClassName(), __FUNCTION__, Logger::format("SearchSet added: %s", id.c_str()));

    // register categories to DB
    auto categories = searchSet->getCategories();
    for (auto& it : categories) {
        Database::getInstance()->adjustOrCreateCategory(it.second);
    }

    return true;
}

bool SearchManager::removeSearchSet(const string& id)
{
    auto it = m_searchSets.find(id);
    if (it == m_searchSets.end()) {
        Logger::warning(getClassName(), __FUNCTION__, Logger::format("Not exist searchSet: %s", id.c_str()));
        return false;
    }

    m_searchSets.erase(id);
    it->second->setClient(nullptr);
    Logger::info(getClassName(), __FUNCTION__, Logger::format("SearchSet removed: %s", id.c_str()));
    return true;
}

void SearchManager::categoryAdded(CategoryPtr category)
{
    Database::getInstance()->adjustOrCreateCategory(std::move(category));
}

void SearchManager::categoryRemoved(const string& cateId)
{
    Database::getInstance()->removeCategory(cateId);
}

SearchSetPtr SearchManager::findSearchSet(const string& id)
{
    auto it = m_searchSets.find(id);
    if (it != m_searchSets.end()) {
        return it->second;
    }
    return nullptr;
}

CategoryPtr SearchManager::findCategory(const string& id)
{
    for (auto& it : m_searchSets) {
        auto searchSet = it.second;
        auto category = searchSet->findCategory(id);
        if (category) {
            return category;
        }
    }
    return nullptr;
}


bool SearchManager::search(const string& searchKey, resultCB callback)
{
    shared_ptr<SearchTask> task = make_shared<SearchTask>(searchKey, callback);

    // from each source
    for (auto& it : m_searchSets) {
        auto& id = it.first;
        auto searchSet = it.second;
        auto source = searchSet->getDataSource();

        // check searchSet's categories are enabled, it's all disabled, don't search the source.
        bool enabled = false;
        for (auto& it : searchSet->getCategories()) {
            if (it.second->isEnabled()) {
                enabled = true;
                break;
            }
        }
        if (!enabled) {
            Logger::info(getClassName(), __FUNCTION__, Logger::format("SearchSet '%s' is disabled totally.", id.c_str()));
            continue;
        }

        // try to search
        source->search(searchKey, [this, task, &searchSet] (const string& sourceId, vector<SearchItemPtr> items) {
            // for each items
            for (auto item : items) {
                const string &cateId = item->getCategory();
                CategoryPtr category = searchSet->findCategory(cateId);
                if (!category) {
                    Logger::warning(getClassName(), __FUNCTION__, Logger::format("Ignore '%s': There is no %s category.",
                        item->getKey().c_str(),
                        cateId.c_str()));
                    continue;
                } else if (!category->isEnabled()) {
                    continue;
                }

                // convert to intent and add to list
                auto intent = category->generateIntent(item);
                task->get(cateId).push_back(intent);

                Logger::debug(getClassName(), __FUNCTION__, Logger::format("Item: %s, %s", cateId.c_str(), item->getKey().c_str()));
            }
        });
    }

    return true;
}

SearchManager::SearchTask::SearchTask(const string& key, resultCB cb)
    : m_key(key)
    , m_callback(std::move(cb))
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

vector<IntentPtr>& SearchManager::SearchTask::get(const string &category)
{
    if (m_intents.find(category) == m_intents.end()) {
        m_intents.insert({category, vector<IntentPtr>()});
    }

    return m_intents[category];
}

void SearchManager::loadPlugins()
{
    auto files = File::readDirectory(PATH_PLUGIN, ".so");
    char *error;
    for (auto& file : files) {
        void *handle = dlopen(File::join(PATH_PLUGIN, file).c_str(), RTLD_LAZY);
        if (!handle) {
            Logger::warning(getClassName(), __FUNCTION__, Logger::format("Failed to load: %s", file.c_str()));
            continue;
        }

        plugin_init* create_plugin = (plugin_init*) dlsym(handle, "create_plugin");
        if (create_plugin == NULL) {
            Logger::warning(getClassName(), __FUNCTION__, "Unknown error");
            dlclose(handle);
            continue;
        } else if ((error = dlerror()) != NULL) {
            Logger::warning(getClassName(), __FUNCTION__, Logger::format("Failed to load init function: %s", error));
            dlclose(handle);
            continue;
        }

        Plugin *plugin = create_plugin();
        addSearchSet(plugin->getSearchSet());

        Logger::info(getClassName(), __FUNCTION__, Logger::format("Plugin loaded: %s", file.c_str()));
        m_pluginHandles.push_back(handle);
    }
}
