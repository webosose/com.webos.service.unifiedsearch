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

#include "conf/ConfFile.h"
#include "bus/client/Configd.h"

ConfFile::ConfFile()
    : m_isRespawned(false)
{
}

ConfFile::~ConfFile()
{
}

bool ConfFile::onInitialization()
{
    loadReadOnlyConf();
    loadReadWriteConf();

    m_isRespawned = File::isFile(this->getRespawnedPath());

    if (!m_isRespawned) {
        File::createFile(this->getRespawnedPath());
    }

    Logger::info(getClassName(), __FUNCTION__,
                 Logger::format("isRespawned(%s)",
                 Logger::toString(m_isRespawned)));

    return true;
}

bool ConfFile::onFinalization()
{
    return true;
}

/** READ ONLY CONFIGS **/

const string& ConfFile::getRespawnedPath()
{
    static string RespawnedPath = "/tmp/search-respawned";
    JValueUtil::getValue(m_readOnlyDatabase, "RespawnedPath", RespawnedPath);
    return RespawnedPath;
}

void ConfFile::loadReadOnlyConf()
{
    m_readOnlyDatabase = JDomParser::fromFile(PATH_RO_SEARCH_CONF);
    if (m_readOnlyDatabase.isNull()) {
        Logger::warning(getClassName(), __FUNCTION__, PATH_RO_SEARCH_CONF, "Failed to parse read-only search-conf");
    }
}

void ConfFile::loadReadWriteConf()
{
    m_readWriteDatabase = JDomParser::fromFile(PATH_RW_SEARCH_CONF);
    if (m_readWriteDatabase.isNull()) {
        m_readWriteDatabase = pbnjson::Object();
        saveReadWriteConf();
    }
}

void ConfFile::saveReadWriteConf()
{
    if (!File::writeFile(PATH_RW_SEARCH_CONF, m_readWriteDatabase.stringify("    ").c_str())) {
        Logger::warning(getClassName(), __FUNCTION__, PATH_RW_SEARCH_CONF, "Failed to save read-write search-conf");
    }
}
