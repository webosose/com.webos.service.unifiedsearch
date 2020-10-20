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

#include "base/DatabaseList.h"

#include "conf/ConfFile.h"
#include "util/File.h"
#include "util/Logger.h"

#define DB_DIR "/var/db/search/"
#define DB_FILE "indexed.db"

DatabaseList::DatabaseList()
{
    setClassName("DatabaseList");
}

DatabaseList::~DatabaseList()
{
}

bool DatabaseList::onInitialization()
{
    // TODO load databases on the folder
    return true;
}

bool DatabaseList::onFinalization()
{
    // TODO shutdown databases
    return true;
}

bool DatabaseList::createDB(string category)
{
    auto db = m_databases.find(category);
    if (db != m_databases.end()) {
        Logger::warning(getClassName(), __func__, Logger::format("Already exist database: %s", category.c_str()));
        return false;
    }

    auto newDB = make_shared<Database>(category);
    m_databases.insert({category, newDB});
    return true;
}

bool DatabaseList::deleteDB(string category)
{
    auto db = m_databases.find(category);
    if (db == m_databases.end()) {
        Logger::warning(getClassName(), __func__, Logger::format("Not exist database: %s", category.c_str()));
        return false;
    }

    m_databases.erase(db);
    return true;
}

DatabasePtr DatabaseList::getDB(string category)
{
    auto db = m_databases.find(category);
    if (db != m_databases.end()) {
        return db->second;
    }
    return nullptr;
}

vector<SearchItemPtr> DatabaseList::search(string searchKey)
{
    return vector<SearchItemPtr>();
}
