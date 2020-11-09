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

#ifndef BUS_CLIENT_SAM_H_
#define BUS_CLIENT_SAM_H_

#include <luna-service2/lunaservice.hpp>
#include <pbnjson.hpp>

#include "Category.h"
#include "LunaClient.h"
#include "SearchSet.h"

#include "clients/AppContents.h"
#include "clients/Applications.h"

#include "interface/ISingleton.h"
#include "Logger.h"
#include "util/JValueUtil.h"

using namespace std;
using namespace LS;
using namespace pbnjson;

class SAM : public LunaClient
          , public ISingleton<SAM> {
friend class ISingleton<SAM>;
public:
    virtual ~SAM();

    bool reloadAppsByLocaleChange();

protected:
    // LunaClient
    virtual void onInitialzed() override;
    virtual void onFinalized() override;
    virtual void onServerStatusChanged(bool isConnected) override;

private:
    static bool onListApps(LSHandle* sh, LSMessage* response, void* context);

    SAM();

    bool addAppContents(JValue &app);

    Call m_listAppsCall;

    SearchSetPtr m_searchSet;
    ApplicationsPtr m_applications;
};

#endif  // BUS_CLIENT_SAM_H_
