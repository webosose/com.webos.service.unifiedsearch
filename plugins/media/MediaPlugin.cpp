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

#include <string>
#include <vector>

#include "MediaPlugin.h"

using namespace std;

MediaPlugin::MediaPlugin()
{
    // create source
    m_source = make_shared<DB8Source>("db8", "com.webos.mediadb");

    // register audio category
    string categoryId = "mediaindexer.audio";
    string categoryName = "Audio Files";
    JValue audioKind = Object();
    audioKind.put("kind", "com.webos.service.mediaindexer.audio:1");
    audioKind.put("targets", Array());
    audioKind["targets"].append("title");
    audioKind["targets"].append("artist");
    auto audio = make_shared<DB8Category>(categoryId, categoryName, audioKind);
    m_source->addKind(categoryId, audioKind);
    m_categories.push_back(audio);

    // create set
    m_searchSet = make_shared<SearchSet>("Media", m_source);
    m_searchSet->addCategory(audio);
}

extern "C" Plugin* create_plugin()
{
    static MediaPlugin *plugin = nullptr;
    if (!plugin) {
        plugin = new MediaPlugin;
    }
    return plugin;
}