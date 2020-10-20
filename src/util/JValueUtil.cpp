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

#include "util/JValueUtil.h"
#include "Environment.h"

map<string, JSchema> JValueUtil::s_schemas;

void JValueUtil::addUniqueItemToArray(pbnjson::JValue& array, string& item)
{
    if (array.isNull() || !array.isArray() || item.empty())
        return;

    for (int i = 0; i < array.arraySize(); ++i) {
        if (array[i].isString() && 0 == item.compare(array[i].asString()))
            return;
    }

    array.append(item);
}

bool JValueUtil::getValue(const JValue& json, const string& key, JValue& value)
{
    if (!json)
        return false;
    if (!json.hasKey(key))
        return false;
    value = json[key];
    return true;
}

bool JValueUtil::getValue(const JValue& json, const string& key, string& value)
{
    if (!json)
        return false;
    if (!json.hasKey(key))
        return false;
    if (!json[key].isString())
        return false;
    if (json[key].asString(value) != CONV_OK) {
        value = "";
        return false;
    }
    return true;
}

bool JValueUtil::getValue(const JValue& json, const string& key, int& value)
{
    if (!json)
        return false;
    if (!json.hasKey(key))
        return false;
    if (!json[key].isNumber())
        return false;
    if (json[key].asNumber<int>(value) != CONV_OK) {
        value = 0;
        return false;
    }
    return true;
}

bool JValueUtil::getValue(const JValue& json, const string& key, bool& value)
{
    if (!json)
        return false;
    if (!json.hasKey(key))
        return false;
    if (!json[key].isBoolean())
        return false;
    if (json[key].asBool(value) != CONV_OK) {
        value = false;
        return false;
    }
    return true;
}

JSchema JValueUtil::getSchema(string name)
{
    if (name.empty())
        return JSchema::AllSchema();

    auto it = s_schemas.find(name);
    if (it != s_schemas.end())
        return it->second;

    string path = PATH_SEARCH_SCHEMAS + name + ".schema";
    pbnjson::JSchema schema = JSchema::fromFile(path.c_str());
    if (!schema.isInitialized())
        return JSchema::AllSchema();

    s_schemas.insert(pair<string, pbnjson::JSchema>(name, schema));
    return schema;
}

bool JValueUtil::hasKey(const JValue& json, const string& firstKey, const string& secondKey, const string& thirdKey)
{
    if (!json.isObject())
        return false;
    if (!json.hasKey(firstKey))
        return false;
    if (!secondKey.empty() && (!json[firstKey].isObject() || !json[firstKey].hasKey(secondKey)))
        return false;
    if (!thirdKey.empty() && (!json[firstKey][secondKey].isObject() || !json[firstKey][secondKey].hasKey(thirdKey)))
        return false;
    return true;
}

string JValueUtil::asString(const JValue& value)
{
    string str = "";
    if (value.isString()) {
        str = value.asString();
    } else if (value.isBoolean()) {
        str = value.asBool() ? "true" : "false";
    } else if (value.isNumber()) {
        str = to_string(value.asNumber<int>());
    }
    return str;
}

string JValueUtil::camelToUnderScore(const JValue& value, const string& prefix)
{
    string underScore = "";
    if (!prefix.empty()) {
        for (uint i = 0; i < prefix.length(); i++) {
            underScore += toupper(prefix[i]);
        }
        underScore += "_";
    }

    string camel = value.asString();
    for (uint i = 0; i < camel.length(); i++) {
        if (isupper(camel[i])) {
            underScore += "_";
        }
        underScore += toupper(camel[i]);
    }
    return underScore;
}
