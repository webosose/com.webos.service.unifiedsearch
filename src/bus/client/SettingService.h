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

#ifndef BUS_CLIENT_SETTINGSERVICE_H_
#define BUS_CLIENT_SETTINGSERVICE_H_

#include <luna-service2/lunaservice.hpp>
#include <boost/signals2.hpp>
#include <pbnjson.hpp>

#include "LunaClient.h"

#include "interface/ISingleton.h"
#include "Logger.h"

using namespace LS;
using namespace pbnjson;

class SettingService : public ISingleton<SettingService>,
                       public LunaClient {
friend class ISingleton<SettingService>;
public:
    virtual ~SettingService();

    const string& localeInfo() const
    {
        return m_localeInfo;
    }

    const string& language() const
    {
        return m_language;
    }

protected:
    // LunaClient
    virtual void onInitialzed() override;
    virtual void onFinalized() override;
    virtual void onServerStatusChanged(bool isConnected) override;

private:
    static bool onLocaleChanged(LSHandle* sh, LSMessage* message, void* context);

    SettingService();

    void updateLocaleInfo(const JValue& localeInfo);

    string m_localeInfo;
    string m_language;

    Call m_getSystemSettingsCall;

};
#endif // BUS_CLIENT_SETTINGSERVICE_H_
