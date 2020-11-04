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

#define QUERY_CATE_TABLE "CREATE TABLE IF NOT EXISTS Category(id, name, intent);"
#define QUERY_CATE_INSERT "INSERT INTO Category values (?, ?, ?);"
#define QUERY_CATE_DELETE_PRE "DELETE FROM Category WHERE id = '"
#define QUERY_CATE_SELECT_PRE "SELECT * FROM Category WHERE id = '"

#define QUERY_ITEM_TABLE "CREATE VIRTUAL TABLE IF NOT EXISTS Items USING FTS3(category, key, text, display, extra);"
#define QUERY_ITEM_INSERT "INSERT INTO Items values (?, ?, ?, ?, ?);"
#define QUERY_ITEM_DELETE_PRE "DELETE FROM Items WHERE category = '"
#define QUERY_ITEM_SELECT_PRE "SELECT * FROM Items WHERE text MATCH '"

Database::Database()
{
    setClassName("Database");
}

bool Database::onInitialization()
{
    // database folder
    File::createDir(PATH_DATABASE);

    // create or open DB file
    string file = File::join(PATH_DATABASE, DATABASE_FILE);
    if (sqlite3_open(file.c_str(), &m_database) != SQLITE_OK) {
        sqlite3_close(m_database);
        Logger::error(getClassName(), __FUNCTION__, Logger::format("Failed to initialize: %s", sqlite3_errmsg(m_database)));
        return false;
    }

    // if it's not exist before, need table
    char *err_msg = nullptr;
    if (sqlite3_exec(m_database, QUERY_ITEM_TABLE, 0, 0, &err_msg) != SQLITE_OK) {
        Logger::error(getClassName(), __FUNCTION__, Logger::format("Failed to create table: %s", err_msg));
        return false;
    }

    if (sqlite3_prepare(m_database, QUERY_ITEM_INSERT, -1, &m_insertStmt, NULL) != SQLITE_OK) {
        Logger::error(getClassName(), __FUNCTION__, "Failed to create 'insert' statement");
        return false;
    }

    Logger::info(getClassName(), __FUNCTION__, "Openning database successed");
    return true;
}

bool Database::onFinalization()
{
    // close DB
    sqlite3_finalize(m_insertStmt);
    sqlite3_close(m_database);
    return true;
}

bool Database::createCategory(CategoryPtr cate)
{
    return true;
}

bool Database::removeCategory(string cateId)
{
    return true;
}

bool Database::insert(SearchItemPtr item)
{
    if (!item) {
        Logger::warning(getClassName(), __FUNCTION__, "Null SearchItem came");
        return false;
    }

    // to use SQLITE_STATIC (don't copy)
    string display = item->getDisplay().stringify();
    string extra = item->getExtra().stringify();

    sqlite3_reset(m_insertStmt);
    sqlite3_bind_text(m_insertStmt, 1, item->getCategory().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(m_insertStmt, 2, item->getKey().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(m_insertStmt, 3, item->getValue().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(m_insertStmt, 4, display.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(m_insertStmt, 5, extra.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(m_insertStmt) != SQLITE_DONE) {
        const char *err_msg = sqlite3_errmsg(m_database);
        Logger::error(getClassName(), __FUNCTION__, Logger::format("Failed to insert: %s - (%s, %s)", err_msg, item->getCategory(), item->getKey()));
        return false;
    }

    Logger::debug(getClassName(), __FUNCTION__, Logger::format("Inserted: %s, %s <= %s", item->getCategory().c_str(), item->getKey().c_str(), item->getValue().c_str()));
    return true;
}

bool Database::remove(string category, string key)
{
    if (category.empty()) {
        Logger::warning(getClassName(), __FUNCTION__, "Category is empty");
        return false;
    }

    char *err_msg = nullptr;
    string query = QUERY_ITEM_DELETE_PRE;
    query += category;
    if (!key.empty()) {
        query += "' AND key = '" + key;
    }
    query += "';";

    if (sqlite3_exec(m_database, query.c_str(), 0, 0, &err_msg) != SQLITE_OK) {
        Logger::error(getClassName(), __FUNCTION__, Logger::format("Failed to delete: %s", err_msg));
        return false;
    }
    Logger::debug(getClassName(), __FUNCTION__, Logger::format("Removed: %s, %s", category.c_str(), key.c_str()));
    return true;
}

vector<SearchItemPtr> Database::search(string searchKey)
{
    vector<SearchItemPtr> searchedItems;

    char *err_msg = nullptr;
    string query = string(QUERY_ITEM_SELECT_PRE) + searchKey + "*';";
    int ret = sqlite3_exec(m_database, query.c_str(), [] (void *data, int n, char **row, char **colNames) -> int {
        vector<SearchItemPtr> *items = static_cast<vector<SearchItemPtr>*>(data);
        if (n > 4 && strlen(row[4]) >= 2) {
            items->push_back(make_shared<SearchItem>(row[0], row[1], row[2], JDomParser::fromString(row[3]), JDomParser::fromString(row[4])));
        } else {
            items->push_back(make_shared<SearchItem>(row[0], row[1], row[2], JDomParser::fromString(row[3])));
        }
        return 0;
    }, &searchedItems, &err_msg);

    if (ret != SQLITE_OK) {
        Logger::error(getClassName(), __FUNCTION__, Logger::format("Failed to search: %s", err_msg));
    }

    return searchedItems;
}