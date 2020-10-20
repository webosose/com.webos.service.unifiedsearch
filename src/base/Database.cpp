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

#include "base/Database.h"

#include "conf/ConfFile.h"
#include "util/File.h"
#include "util/Logger.h"

#define DB_DIR "/var/db/search/"
#define DB_FILE "indexed.db"

Database::Database(string category) : m_category(category)
{
    setClassName("Database");
}

Database::~Database()
{
}

bool Database::onInitialization() {
    // ensure directory
    File::createDir(DB_DIR);

    // create or open DB file
    if (sqlite3_open(DB_DIR DB_FILE, &m_database) != SQLITE_OK) {
        Logger::error(getClassName(), __FUNCTION__, "Failed to open DB");
        sqlite3_close(m_database);
        return false;
    }

    Logger::info(getClassName(), __FUNCTION__, "Openning database successed.");
    return true;
}

bool Database::onFinalization() {
    // close DB
    sqlite3_close(m_database);
    return true;
}
