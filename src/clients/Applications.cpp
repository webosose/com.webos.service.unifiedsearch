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

#include "Applications.h"

#include "base/Database.h"
#include "base/SearchManager.h"
#include "util/File.h"
#include "util/JValueUtil.h"

Applications::Applications() : Category("Applications")
{
    // remove old items first
    // TODO: it's better to update(replace) only updated one - needs better logic
    Database::getInstance()->removeItem(getCategoryId());
}

Applications::~Applications()
{
}

bool Applications::addToDatabase(JValue &app)
{
    bool visible;
    if (JValueUtil::getValue(app, "visible", visible) && !visible) {
        // ignore non-visible apps
        return false;
    }

    string id, title, icon, folderPath;
    JValueUtil::getValue(app, "id", id);
    JValueUtil::getValue(app, "title", title);
    JValueUtil::getValue(app, "folderPath", folderPath);
    JValueUtil::getValue(app, "icon", icon);

    JValue display = Object();
    display.put("title", title);
    display.put("icon", File::join(folderPath, icon));

    // create search item and insert
    SearchItemPtr item = make_shared<SearchItem>(getCategoryId(), id, title, display);
    return Database::getInstance()->insertItem(item);
}

IntentPtr Applications::generateIntent(SearchItemPtr item)
{
    auto intent = make_shared<Intent>(getCategoryId());

/*
    // For now, we using explicit intent because intent manager doesn't accept service as a intent handler.
    intent->setAction("launch");
    intent->setUri(string("app://") + item->getKey());
*/

    // to create explicit intent
    intent->getBase().put("name", item->getKey());

    // set intent
    intent->setDisplay(item->getDisplay());

    return intent;
}

bool Applications::removeFromDatabase(string id)
{
    return Database::getInstance()->removeItem(getCategoryId(), id);
}