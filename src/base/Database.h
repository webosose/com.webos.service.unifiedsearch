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

#ifndef BASE_DATABASE_H_
#define BASE_DATABASE_H_

#include <map>
#include <string>
#include <sqlite3.h>
#include <pbnjson.hpp>

#include "interface/IClassName.h"
#include "interface/ISingleton.h"
#include "interface/IInitializable.h"

using namespace std;

class Database: public IInitializable {
friend class ISingleton<Database>;
public:
    Database(string category);
    virtual ~Database();

    bool onInitialization();
    bool onFinalization();

    bool insert() { return true; }
    bool remove() { return true; }

private:
    string m_category;
    sqlite3 *m_database;
};

typedef shared_ptr<Database> DatabasePtr;

#endif /* BASE_DATABASE_H_ */
