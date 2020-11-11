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

#ifndef __CONF_CONF_FILE_H__
#define __CONF_CONF_FILE_H__

#include <string>
#include <pbnjson.hpp>
#include <boost/signals2.hpp>
#include <pwd.h>
#include <sys/types.h>

#include "Environment.h"
#include "interface/IInitializable.h"
#include "interface/ISingleton.h"
#include "util/File.h"
#include "util/JValueUtil.h"
#include "Logger.h"
#include "util/LinuxProcess.h"

class ConfFile : public ISingleton<ConfFile>,
                 public IInitializable<ConfFile> {
friend class ISingleton<ConfFile>;
public:
    virtual ~ConfFile();

    // IInitializable
    virtual bool onInitialization();
    virtual bool onFinalization();

    /** READ ONLY CONFIGS **/
    bool isRespawned()
    {
        return m_isRespawned;
    }

    const string& getRespawnedPath();
    const string& getLoginBrokerEnablerPath();

    /** READ WRIETE CONFIGS **/

private:
    ConfFile();

    void loadReadOnlyConf();
    void loadReadWriteConf();
    void saveReadWriteConf();

    JValue m_readOnlyDatabase;
    JValue m_readWriteDatabase;

    bool m_isRespawned;
};

#endif // __CONF_CONF_FILE_H__

