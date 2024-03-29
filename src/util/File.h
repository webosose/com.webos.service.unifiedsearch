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

#ifndef UTIL_FILE_H_
#define UTIL_FILE_H_

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>

using namespace std;

class File {
public:
    static void set_slash_to_base_path(string& path);
    static string readFile(const string& file_name);
    static bool writeFile(const string& filePath, const string& buffer);
    static bool concatToFilename(const string originPath, string& returnPath, const string addingStr);

    static bool isDirectory(const string& path);
    static bool isFile(const string& path);
    static bool createDir(const string& path);
    static bool createFile(const string& path);

    static vector<string> readDirectory(const string& path, const string& filter = "");

    static bool chownDir(const string& path, const string& userName);

    static string join(const string& a, const string& b);
    static vector<string> splitPath(const string& path);

    static void trimPath(string &path)
    {
        if (path.back() == '/')
            path.erase(prev(path.end()));
    }

    File();
    virtual ~File();

};

#endif /* UTIL_FILE_H_ */
