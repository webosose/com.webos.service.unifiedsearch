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

#ifndef BASE_CATEGORYLIST_H_
#define BASE_CATEGORYLIST_H_

#include <map>
#include <vector>
#include <string>
#include <sqlite3.h>
#include <pbnjson.hpp>

#include "base/Category.h"
#include "base/Intent.h"
#include "base/SearchItem.h"

#include "interface/IClassName.h"
#include "interface/ISingleton.h"
#include "interface/IInitializable.h"

using namespace std;

class CategoryList : public IInitializable
                   , public ISingleton<CategoryList> {
friend class ISingleton<CategoryList>;
public:
    virtual ~CategoryList();

    bool onInitialization();
    bool onFinalization();

    bool addCategory(CategoryPtr category);
    bool removeCategory(string name);

    CategoryPtr find(string name);
    vector<IntentPtr> search(string searchKey);

private:
    CategoryList();

    map<string, CategoryPtr> m_categories;
};

#endif /* BASE_DATABASE_H_ */
