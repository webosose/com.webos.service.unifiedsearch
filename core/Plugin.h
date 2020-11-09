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

#ifndef BASE_CORE_PLUGIN_H_
#define BASE_CORE_PLUGIN_H_

#include <string>
#include <vector>

#include "SearchSet.h"

using namespace std;

class Plugin {
public:
    Plugin() {}
    virtual ~Plugin() {}

    virtual SearchSetPtr getSearchSet() = 0;
    virtual const string& getName() = 0;
};

typedef Plugin* plugin_init();

extern "C" Plugin* create_plugin();

#endif /* BASE_PLUGIN_H_ */