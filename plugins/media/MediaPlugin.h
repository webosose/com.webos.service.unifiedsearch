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

#ifndef PLUGIN_AUDIO_H_
#define PLUGIN_AUDIO_H_

#include <string>
#include <vector>

#include "Plugin.h"
#include "SearchSet.h"

#include "DB8Source.h"
#include "DB8Category.h"

using namespace std;

class MediaPlugin : public Plugin {
public:
    MediaPlugin();
    ~MediaPlugin() {}

    SearchSetPtr getSearchSet() override { return m_searchSet; }
    const string& getName() override { return m_name; };

private:
    SearchSetPtr m_searchSet;

    string m_name;
    DB8SourcePtr m_source;
    vector<CategoryPtr> m_categories;
};

#endif /* PLUGIN_AUDIO_H_ */