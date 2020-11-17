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

#ifndef MAIN_DAEMON_H
#define MAIN_DAEMON_H

#include <pbnjson.hpp>
#include <luna-service2/lunaservice.h>

#include "interface/ISingleton.h"
#include "interface/IClassName.h"

using namespace std;
using namespace pbnjson;

class MainDaemon : public ISingleton<MainDaemon>,
                   public IClassName<MainDaemon> {
friend class ISingleton<MainDaemon>;
public:
    virtual ~MainDaemon();

    void initialize();
    void finalize();

    void start();
    void stop();

private:
    MainDaemon();

    GMainLoop *m_mainLoop;
};

#endif
