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

#include <pbnjson.hpp>

#include "base/Database.h"

#include "conf/ConfFile.h"
#include "util/File.h"
#include "util/Logger.h"

Database::Database()
{
    setClassName("Database");
}

bool Database::onInitialization()
{
    // database folder
    File::createDir(PATH_DATABASE);

    // check already created
    string file = File::join(PATH_DATABASE, DATABASE_FILE);
    bool alreadyExist = File::isFile(file);

    // create or open DB file
    if (sqlite3_open(file.c_str(), &m_database) != SQLITE_OK) {
        sqlite3_close(m_database);
        Logger::error(getClassName(), __FUNCTION__, Logger::format("Failed to initialize: %s", sqlite3_errmsg(m_database)));
        return false;
    }

    // if it's not exist before, need table
    if (!alreadyExist) {
        char *err_msg = nullptr;
        const char *query = "CREATE VIRTUAL TABLE Items USING FTS3(category, key, text, extra);";
        if (sqlite3_exec(m_database, query, 0, 0, &err_msg) != SQLITE_OK) {
            Logger::error(getClassName(), __FUNCTION__, Logger::format("Failed to create table: %s", err_msg));
            return false;
        }
    }

    Logger::info(getClassName(), __FUNCTION__, "Openning database successed");
    return true;
}

bool Database::onFinalization()
{
    // close DB
    sqlite3_close(m_database);
    return true;
}

bool Database::insert(SearchItemPtr item)
{
    if (!item) {
        Logger::warning(getClassName(), __FUNCTION__, "Null SearchItem came");
        return false;
    }

    char *err_msg = nullptr;
    sqlite3_stmt *stmt;
    string query = "INSERT INTO Items values (?, ?, ?, ?);";
    if (sqlite3_prepare(m_database, query.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        Logger::error(getClassName(), __FUNCTION__, "Failed to create statement");
        return false;
    }

    sqlite3_bind_text(stmt, 1, item->getCategory().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, item->getKey().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, item->getValue().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, item->getExtra().stringify().c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        Logger::error(getClassName(), __FUNCTION__, Logger::format("Failed to insert: %s, %s", err_msg, query.c_str()));
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool Database::remove(string category, string key)
{
    if (category.empty() || key.empty()) {
        Logger::warning(getClassName(), __FUNCTION__, "Category or key is empty");
        return false;
    }

    char *err_msg = nullptr;
    string query = "DELETE FROM Items WHERE ";
    // category, key, text, extra);";
    query += "category = '" + category + "' AND ";
    query += "key = '" + key + "';";

    if (sqlite3_exec(m_database, query.c_str(), 0, 0, &err_msg) != SQLITE_OK) {
        Logger::error(getClassName(), __FUNCTION__, Logger::format("Failed to delete: %s", err_msg));
        return false;
    }
    return true;
}

vector<SearchItemPtr> Database::search(string searchKey)
{
    vector<SearchItemPtr> searchedItems;

    char *err_msg = nullptr;
    string query = "select * from Items where text match '" + searchKey + "*';";
    int ret = sqlite3_exec(m_database, query.c_str(), [] (void *data, int n, char **row, char **colNames) -> int {
        vector<SearchItemPtr> *items = static_cast<vector<SearchItemPtr>*>(data);
        items->push_back(make_shared<SearchItem>(row[0], row[1], row[2], row[3]));
        return 0;
    }, &searchedItems, &err_msg);

    if (ret != SQLITE_OK) {
        Logger::error(getClassName(), __FUNCTION__, Logger::format("Failed to search: %s", err_msg));
    }

    return searchedItems;
}