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

#ifndef BUS_CLIENT_APPLICATIONS_H_
#define BUS_CLIENT_APPLICATIONS_H_

#include <luna-service2/lunaservice.hpp>
#include <pbnjson.hpp>

#include "base/Category.h"

#include "interface/IClassName.h"
#include "interface/ISingleton.h"
#include "util/Logger.h"

using namespace std;
using namespace LS;
using namespace pbnjson;

class Applications : public Category
                   , public IClassName  {
public:
    Applications();
    virtual ~Applications();

    bool addToDatabase(JValue &app);
    bool removeFromDatabase(string &id);

    IntentPtr generateIntent(SearchItemPtr item);
};

#endif  // BUS_CLIENT_Applications_H_
