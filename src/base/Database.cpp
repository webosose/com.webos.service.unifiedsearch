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
#include "base/SearchManager.h"

#include "conf/ConfFile.h"
#include "util/File.h"
#include "Logger.h"

static const map<string, string> tableQueries = {
    { "ITEM", "CREATE VIRTUAL TABLE IF NOT EXISTS Items USING FTS3(category, key, text, display, extra);" },
    { "CATEGORY", "CREATE TABLE IF NOT EXISTS Category(id TEXT PRIMARY KEY, name TEXT, rank INTEGER, enabled INTEGER);" }
};

static const map<string, string> statementQueries = {
    { "ITEM_INSERT",     "INSERT INTO Items values (?, ?, ?, ?, ?);" },
    { "ITEM_SELECT",     "SELECT * FROM Items WHERE text MATCH ?" },
    { "CATE_INSERT",     "INSERT INTO Category values (?, ?, ?, 1);" },
    { "CATE_UPDATE",     "UPDATE Category set rank = ?, enabled = ?, name = ? where id = ?;" },
    { "CATE_DELETE",     "DELETE FROM Category WHERE id = ?;" },
    { "CATE_SELECT",     "SELECT * FROM Category WHERE id = ?;" },
    { "CATE_RANK",       "SELECT * FROM Category ORDER BY rank ASC;" },
    { "CATE_MAXRANK",    "SELECT max(rank) FROM Category WHERE enabled = 1;" },
    { "CATE_CHANGERANK", "UPDATE Category SET rank = rank + ? WHERE enabled = 1 AND rank >= ? AND rank <= ?;" }
};

static const map<string, string> normalQueries = {
    { "ITEM_DELETE", "DELETE FROM Items WHERE category = '" }
};

Database::Database() : DataSource("sqlite3"), m_database(nullptr)
{
}

Database::~Database()
{
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
    for (auto it : tableQueries) {
        if (sqlite3_exec(m_database, it.second.c_str(), 0, 0, &err_msg) != SQLITE_OK) {
            Logger::error(getClassName(), __FUNCTION__, Logger::format("Failed to create item table '%s': %s", it.first.c_str(), err_msg));
            if (err_msg) {
                sqlite3_free(err_msg);
            }
            return false;
        }
    }

    // create statements
    for (auto it : statementQueries) {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare(m_database, it.second.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
            Logger::error(getClassName(), __FUNCTION__, "Failed to create '%s' statement", it.first.c_str());
            return false;
        }
        m_statements.insert({it.first, stmt});
    }

    Logger::info(getClassName(), __FUNCTION__, "Openning database successed");
    return true;
}

bool Database::onFinalization()
{
    // close DB
    for (auto it : m_statements) {
        sqlite3_finalize(it.second);
    }
    sqlite3_close(m_database);
    return true;
}

bool Database::adjustOrCreateCategory(CategoryPtr cate)
{
    if (!cate) {
        Logger::warning(getClassName(), __FUNCTION__, "Null category came");
        return false;
    }

    const char* id = cate->getCategoryId().c_str();
    const char* name = cate->getCategoryName().c_str();

    // check it's already exist
    auto stmt = m_statements["CATE_SELECT"];
    sqlite3_reset(stmt);
    sqlite3_bind_text(stmt, 1, id, -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // already exist get columns
        name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        int rank = sqlite3_column_int(stmt, 2);
        int enabled = sqlite3_column_int(stmt, 3);
        // adjust category info from DB
        cate->setCategoryName(name);
        cate->setRank(rank);
        cate->setEnabled(enabled);
        Logger::info(getClassName(), __FUNCTION__, Logger::format("Adjustted: Category (%s, '%s', %d, %s)", id, name, rank, (enabled ? "Y" : "N")));
    } else {
        // not exist, get add max rank first
        int rank = RANK_MAX;
        stmt = m_statements["CATE_MAXRANK"];
        sqlite3_reset(stmt);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            // already exist get columns
            rank = sqlite3_column_int(stmt, 0) + 1;
        }

        // add category
        stmt = m_statements["CATE_INSERT"];
        sqlite3_reset(stmt);
        sqlite3_bind_text(stmt, 1, id, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, name, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, rank);
        cate->setRank(rank);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            const char *err_msg = sqlite3_errmsg(m_database);
            Logger::error(getClassName(), __FUNCTION__, Logger::format("Failed to insert category: %s - (%s, %s)", err_msg, id, name));
            return false;
        }

        Logger::info(getClassName(), __FUNCTION__, Logger::format("Inserted: Category (%s, '%s')", id, name));
    }

    return true;
}

bool Database::removeCategory(string cateId)
{
    if (cateId.empty()) {
        Logger::warning(getClassName(), __FUNCTION__, "Empty category id");
        return false;
    }

    auto stmt = m_statements["CATE_REMOVE"];
    sqlite3_reset(stmt);
    sqlite3_bind_text(stmt, 1, cateId.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        const char *err_msg = sqlite3_errmsg(m_database);
        Logger::error(getClassName(), __FUNCTION__, Logger::format("Failed to remove category: %s - %s", err_msg, cateId.c_str()));
        return false;
    }

    // remove items also
    removeItem(cateId);

    Logger::info(getClassName(), __FUNCTION__, Logger::format("Removed: Category %s", cateId.c_str()));
    return true;
}

bool Database::updateCategory(CategoryPtr cate)
{
    if (!cate) {
        Logger::warning(getClassName(), __FUNCTION__, "Null category came");
        return false;
    }

    const string &id = cate->getCategoryId();
    string name = cate->getCategoryName();

    int oldRank = -1;
    bool oldEnabled = false;

    // before update, get current DB data to order rank correctly
    int countEnabledCate = 0;
    auto stmt = m_statements["CATE_RANK"];
    sqlite3_reset(stmt);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *cateId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        int rank = sqlite3_column_int(stmt, 2);
        bool enabled = sqlite3_column_int(stmt, 3) > 0;
        if (enabled) {
            countEnabledCate++;
        }
        if (id == cateId) {
            oldRank = rank;
            oldEnabled = enabled;
            // if no name entered, use previous one
            if (name.empty()) {
                name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            }
            // if disable => enable case, it should be cared enabled one for count
            if (!oldEnabled && cate->isEnabled()) {
                countEnabledCate++;
            }
        }
    }

    // if oldRank is not updated, there is no matched category ID.
    if (oldRank < 0) {
        Logger::error(getClassName(), __FUNCTION__, Logger::format("No matched category on DB: %s", id.c_str()));
        return false;
    }

    // re-range the rank (correct user mistake)
    int rank = min(max(cate->getRank(), 1), countEnabledCate);
    bool enabled = cate->isEnabled();

    // change other categories ranks when needs
    if (oldEnabled != enabled || (enabled && oldRank != cate->getRank())) {
        if (enabled) {
            if (oldRank < rank) {
                updateRanks(-1, oldRank + 1, rank);
            } else {
                updateRanks(1, rank, oldRank - 1);
            }
        } else {
            // going to disable
            // #1, set it's rank as big value
            rank = RANK_MAX;
            // #2, set rank-- for other intermediate categories
            updateRanks(-1, oldRank, RANK_MAX);
        }
    }

    // update itself
    stmt = m_statements["CATE_UPDATE"];
    sqlite3_reset(stmt);
    sqlite3_bind_int(stmt, 1, rank);
    sqlite3_bind_int(stmt, 2, enabled);
    sqlite3_bind_text(stmt, 3, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, id.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        const char *err_msg = sqlite3_errmsg(m_database);
        Logger::error(getClassName(), __FUNCTION__, Logger::format("Failed to update category: %s - (%s, %s)", err_msg, id.c_str(), name.c_str()));
        return false;
    }

    Logger::info(getClassName(), __FUNCTION__, Logger::format("Updated: Category (%s, '%s', %d, %s)", id.c_str(), name.c_str(), rank, (enabled ? "Y" : "N")));

    return true;
}

vector<CategoryPtr> Database::getCategories() {
    vector<CategoryPtr> categories;

    auto stmt = m_statements["CATE_RANK"];
    sqlite3_reset(stmt);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        // already exist get columns
        const char* id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        int rank = sqlite3_column_int(stmt, 2);
        int enabled = sqlite3_column_int(stmt, 3);
        auto category = make_shared<Category>(id, name);
        category->setRank(rank);
        category->setEnabled(enabled);
        categories.push_back(category);
    }

    return categories;
}

bool Database::updateRanks(int value, int start, int end)
{
    auto stmt = m_statements["CATE_CHANGERANK"];
    sqlite3_reset(stmt);
    sqlite3_bind_int(stmt, 1, value);
    sqlite3_bind_int(stmt, 2, start);
    sqlite3_bind_int(stmt, 3, end);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        const char *err_msg = sqlite3_errmsg(m_database);
        Logger::error(getClassName(), __FUNCTION__, Logger::format("Failed to update ranks: %s - (%s, %d-%d)", err_msg, start, end));
        return false;
    }
    Logger::info(getClassName(), __FUNCTION__, Logger::format("Updated : ranks (%d-%d) %d", start, end, value));
    return true;
}

bool Database::insertItem(SearchItemPtr item)
{
    if (!item) {
        Logger::warning(getClassName(), __FUNCTION__, "Null SearchItem came");
        return false;
    }

    // to use SQLITE_STATIC (don't copy)
    string display = item->getDisplay().stringify();
    string extra = item->getExtra().stringify();

    auto stmt = m_statements["ITEM_INSERT"];
    sqlite3_reset(stmt);
    sqlite3_bind_text(stmt, 1, item->getCategory().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, item->getKey().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, item->getValue().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, display.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, extra.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        const char *err_msg = sqlite3_errmsg(m_database);
        Logger::error(getClassName(), __FUNCTION__, Logger::format("Failed to insert: %s - (%s, %s)", err_msg, item->getCategory(), item->getKey()));
        return false;
    }

    Logger::debug(getClassName(), __FUNCTION__, Logger::format("Inserted: %s, %s <= %s", item->getCategory().c_str(), item->getKey().c_str(), item->getValue().c_str()));
    return true;
}

bool Database::removeItem(string category, string key)
{
    if (category.empty()) {
        Logger::warning(getClassName(), __FUNCTION__, "Category is empty");
    }

    char *err_msg = nullptr;
    string query = normalQueries.at("ITEM_DELETE");
    query += category;
    if (!key.empty()) {
        query += "' AND key = '" + key;
    }
    query += "';";

    if (sqlite3_exec(m_database, query.c_str(), 0, 0, &err_msg) != SQLITE_OK) {
        Logger::error(getClassName(), __FUNCTION__, Logger::format("Failed to delete: %s", err_msg));
        if (err_msg) {
            sqlite3_free(err_msg);
        }
        return false;
    }
    Logger::debug(getClassName(), __FUNCTION__, Logger::format("Removed: %s, %s", category.c_str(), key.c_str()));
    return true;
}

bool Database::search(string searchKey, searchCB callback)
{
    vector<SearchItemPtr> searchedItems;

    auto stmt = m_statements["ITEM_SELECT"];
    auto key = searchKey + "*";
    sqlite3_reset(stmt);
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* cateId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* key = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* display = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        const char* extra = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

        SearchItemPtr item;
        JValue dispObj = JDomParser::fromString(display);
        if (extra && strlen(extra) >= 2) {
            item = make_shared<SearchItem>(cateId, key, value, dispObj, JDomParser::fromString(extra));
        } else {
            item = make_shared<SearchItem>(cateId, key, value, dispObj);
        }
        searchedItems.push_back(item);
    }

    Logger::info(getClassName(), __FUNCTION__, Logger::format("Find '%s' => %d item(s) on %s", searchKey.c_str(), searchedItems.size(), getId().c_str()));
    callback(getId(), searchedItems);
    return true;
}
