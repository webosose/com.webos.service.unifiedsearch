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

#include "File.h"

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

void File::set_slash_to_base_path(string& path)
{
    if (!path.empty() && path[path.length() - 1] == '/')
        return;

    path.append("/");
}

string File::readFile(const string& file_name)
{
    ifstream file(file_name.c_str(), ifstream::in);
    string file_contents;

    if (file.is_open() && file.good()) {
        stringstream buf;
        buf << file.rdbuf();
        file_contents = buf.str();
    }

    return file_contents;
}

bool File::writeFile(const string &path, const string& buffer)
{
    ofstream file(path.c_str());
    if (file.is_open()) {
        file << buffer;
        file.close();
    } else {
        return false;
    }
    return true;
}

bool File::concatToFilename(const string originPath, string& returnPath, const string addingStr)
{
    if (originPath.empty() || addingStr.empty())
        return false;

    returnPath = "";

    string dir_path, filename, name_only, ext;
    size_t pos_dir = originPath.find_last_of("/");

    if (string::npos == pos_dir) {
        filename = originPath;
    } else {
        pos_dir = pos_dir + 1;
        dir_path = originPath.substr(0, pos_dir);
        filename = originPath.substr(pos_dir);
    }

    size_t pos_ext = filename.find_last_of(".");

    if (string::npos == pos_ext)
        return false;

    name_only = filename.substr(0, pos_ext);
    ext = filename.substr(pos_ext);

    if (ext.length() < 2)
        return false;

    returnPath = dir_path + name_only + addingStr + ext;

    return true;
}

bool File::isDirectory(const string& path)
{
    struct stat dirStat;
    if (stat(path.c_str(), &dirStat) != 0 || (dirStat.st_mode & S_IFDIR) == 0) {
        return false;
    }
    return true;
}

bool File::isFile(const string& path)
{
    struct stat fileStat;

    if (stat(path.c_str(), &fileStat) != 0 || (fileStat.st_mode & S_IFREG) == 0) {
        return false;
    }
    return true;
}

bool File::createDir(const string& path)
{
    const char* path_str = path.c_str();
    char temp[100];

    int pos = 0;
    const int len = path.size();
    struct stat info;
    while (pos <= len) {
        if ((path_str[pos] == '/' || path_str[pos] == 0) && pos != 0) {
            strncpy(temp, path_str, pos);
            temp[pos] = 0;
            if (mkdir(temp, 0755) != 0) {
                if (stat(temp, &info) != 0 || !S_ISDIR(info.st_mode)) {
                    // error: file exist, cannot create directory
                    return false;
                }
            }
        }
        pos++;
    }
    return true;
}

bool File::createFile(const string& path)
{
    return File::writeFile(path, "");
}

vector<string> File::readDirectory(const string& path, const string& filter)
{
    vector<string> files;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (path.c_str())) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            string dirName = ent->d_name;
            if (filter.empty() || (dirName.find(filter) != string::npos)) {
                files.push_back(dirName);
            }
        }
        closedir (dir);
    }
    return files;
}


bool File::chownDir(const string& path, const string& userName)
{
    struct passwd *user;
    user = getpwnam(userName.c_str());
    if (user == NULL || chown(path.c_str(), user->pw_uid, user->pw_gid) < 0) {
        return false;
    }
    return true;
}

string File::join(const string& a, const string& b)
{
    string path = "";

    if (a.back() == '/') {
        if (b.front() == '/') {
            path = a + b.substr(1);
        }
        else {
            path = a + b;
        }
    } else {
        if (b.front() == '/') {
            path = a + b;
        }
        else {
            path = a + "/" + b;
        }
    }
    return path;
}

vector<string> File::splitPath(const string& path)
{
    vector<string> result;
    string token;
    size_t start = 0;
    size_t end = path.find("/");
    while (end != string::npos) {
        token = path.substr(start, end-start);
        if (!token.empty())
            result.push_back(token);
        start = end + 1;
        end = path.find("/", start);
    }
    token = path.substr(start);
    result.push_back(token);
    return result;
}

File::File()
{
}

File::~File()
{
}
