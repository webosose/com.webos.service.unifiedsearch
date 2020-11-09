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

#ifndef BASE_CORE_DATASOURCE_H_
#define BASE_CORE_DATASOURCE_H_

#include <string>
#include <vector>
#include <functional>

#include "SearchItem.h"

using namespace std;

class DataSource {
public:
    DataSource(string id) : m_id(id) {}
    virtual ~DataSource() {}

    string getId() { return m_id; }

    using searchCB = function<void(string, vector<SearchItemPtr>)>;
    virtual bool search(string searchKey, searchCB callback) = 0;

private:
    string m_id;
};

typedef shared_ptr<DataSource> DataSourcePtr;

#endif /* BASE_DATASOURCE_H_ */