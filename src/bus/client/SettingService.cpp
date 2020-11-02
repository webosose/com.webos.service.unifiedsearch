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

#include "SettingService.h"

#include <stdlib.h>

#include "bus/service/UnifiedSearch.h"
#include "util/JValueUtil.h"

SettingService::SettingService()
    : AbsLunaClient("com.webos.settingsservice")
{
    setClassName("SettingService");
}

SettingService::~SettingService()
{
}

void SettingService::onInitialzed()
{

}

void SettingService::onFinalized()
{
    m_getSystemSettingsCall.cancel();
}

void SettingService::onServerStatusChanged(bool isConnected)
{
    static string method = string("luna://") + getName() + string("/getSystemSettings");

    if (isConnected) {
        JValue requestPayload = pbnjson::Object();
        requestPayload.put("subscribe", true);
        requestPayload.put("key", "localeInfo");

        m_getSystemSettingsCall = UnifiedSearch::getInstance().callMultiReply(
            method.c_str(),
            requestPayload.stringify().c_str(),
            onLocaleChanged,
            nullptr
        );
    } else {
        m_getSystemSettingsCall.cancel();
    }
}

bool SettingService::onLocaleChanged(LSHandle* sh, LSMessage* message, void* context)
{
    Message response(message);
    JValue subscriptionPayload = JDomParser::fromString(response.getPayload());
    Logger::logSubscriptionResponse(getInstance().getClassName(), __FUNCTION__, response, subscriptionPayload);

    if (subscriptionPayload.isNull())
        return true;

    bool returnValue = true;
    if (!JValueUtil::getValue(subscriptionPayload, "returnValue", returnValue) || !returnValue) {
        Logger::warning(getInstance().getClassName(), __FUNCTION__, "received invaild message from settings service");
        return true;
    }

    if (!subscriptionPayload["settings"].isObject()) {
        return true;
    }

    SettingService::getInstance().updateLocaleInfo(subscriptionPayload["settings"]);
    return true;
}

void SettingService::updateLocaleInfo(const JValue& settings)
{
    string localeInfo;

    if (settings.isNull() || !settings.isObject()) {
        return;
    }

    if (!JValueUtil::getValue(settings, "localeInfo", "locales", "UI", localeInfo)) {
        return;
    }

    if (localeInfo.empty() || localeInfo == m_localeInfo) {
        return;
    }

    m_localeInfo = localeInfo;
    if (!m_localeInfo.empty()) {
        m_language = m_localeInfo.substr(0, m_localeInfo.find("-"));
    }
}