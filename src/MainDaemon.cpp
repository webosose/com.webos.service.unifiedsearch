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

#include "MainDaemon.h"

#include <glib.h>

#include "base/Database.h"
#include "base/SearchManager.h"
#include "bus/service/UnifiedSearch.h"
#include "bus/client/Configd.h"
#include "bus/client/SAM.h"
#include "bus/client/SettingService.h"

#include "conf/ConfFile.h"
#include "util/JValueUtil.h"

MainDaemon::MainDaemon()
{
    setClassName("MainDaemon");
    m_mainLoop = g_main_loop_new(NULL, FALSE);
}

MainDaemon::~MainDaemon()
{
    if (m_mainLoop) {
        g_main_loop_unref(m_mainLoop);
    }
}

void MainDaemon::initialize()
{
    ConfFile::getInstance()->initialize();
    Database::getInstance()->initialize();
    UnifiedSearch::getInstance()->initialize(m_mainLoop);

    SettingService::getInstance()->initialize();
    SAM::getInstance()->initialize();
    SearchManager::getInstance()->initialize();
}

void MainDaemon::finalize()
{
    SearchManager::getInstance()->finalize();
    SAM::getInstance()->finalize();
    SettingService::getInstance()->finalize();
    
    Database::getInstance()->finalize();
    ConfFile::getInstance()->finalize();
}

void MainDaemon::start()
{
    Logger::info(getClassName(), __FUNCTION__, "Start event handler thread");

    g_main_loop_run(m_mainLoop);
}

void MainDaemon::stop()
{
    if (m_mainLoop)
        g_main_loop_quit(m_mainLoop);
}
