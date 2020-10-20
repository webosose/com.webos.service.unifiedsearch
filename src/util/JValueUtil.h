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

#ifndef UTIL_JVALUEUTIL_H_
#define UTIL_JVALUEUTIL_H_

#include <iostream>
#include <map>
#include <pbnjson.hpp>

using namespace std;
using namespace pbnjson;

class JValueUtil {
public:
    JValueUtil() {}
    virtual ~JValueUtil() {}

    static void addUniqueItemToArray(JValue& arr, string& str);

    template <typename T>
    static bool getValue(const JValue& json, const string& firstKey, const string& secondKey, const string& thirdKey, T& value)
    {
        if (!json)
            return false;
        if (firstKey.empty())
            return getValue(json, secondKey, thirdKey, value);
        if (!json.hasKey(firstKey))
            return false;
        if (!json[firstKey].isObject())
            return false;
        return getValue(json[firstKey], secondKey, thirdKey, value);
    }

    template <typename T>
    static bool getValue(const JValue& json, const string& firstKey, const string& secondKey, T& value)
    {
        if (!json)
            return false;
        if (firstKey.empty())
            return getValue(json, secondKey, value);
        if (!json.hasKey(firstKey))
            return false;
        if (!json[firstKey].isObject())
            return false;
        return getValue(json[firstKey], secondKey, value);
    }

    static bool getValue(const JValue& json, const string& key, JValue& value);
    static bool getValue(const JValue& json, const string& key, string& value);
    static bool getValue(const JValue& json, const string& key, int& value);
    static bool getValue(const JValue& json, const string& key, bool& value);

    static JSchema getSchema(string name);

    static bool hasKey(const JValue& json, const string& firstKey, const string& secondKey = "", const string& thirdKey = "");

    static string asString(const JValue& value);
    static string camelToUnderScore(const JValue& value, const string& prefix = "");

private:
    static map<string, JSchema> s_schemas;
};

#endif /* UTIL_JVALUEUTIL_H_ */
