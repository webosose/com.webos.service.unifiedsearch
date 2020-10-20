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

#ifndef BASE_SEARCHITEM_H_
#define BASE_SEARCHITEM_H_

#include <map>
#include <string>
#include <pbnjson.hpp>

#include "interface/IClassName.h"
#include "interface/ISerializable.h"

using namespace std;
using namespace pbnjson;

class SearchItem: public IClassName {
public:
    SearchItem();
    virtual ~SearchItem();

    int getId() { return m_id; }
    const string& getKey() { return m_key; }
    JValue& getValue() { return m_value; }

private:
    int m_id;
    string m_key;
    JValue m_value;
};

typedef shared_ptr<SearchItem> SearchItemPtr;

#endif /* BASE_SEARCHITEM_H_ */