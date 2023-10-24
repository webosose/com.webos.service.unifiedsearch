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

#ifndef INTERFACE_ICLASSNAME_H_
#define INTERFACE_ICLASSNAME_H_

#include <string>
#include <cstdlib>
#include <cxxabi.h>

using namespace std;

template<class T>
class IClassName {
public:
    IClassName() {
        int status;
        s_name = typeid(T).name();
        if(s_name == NULL) s_name = "";
        char *demangled_name = abi::__cxa_demangle(s_name.c_str(), NULL, NULL, &status);
        std::string d_name(demangled_name ? demangled_name : "");
        if (status == 0) {
            s_name = d_name;
            std::free(demangled_name);
        }
    }

    static const string& getClassName()
    {
        return s_name;
    }

private:
    static string s_name;
};

template<class T>
string IClassName<T>::s_name = "";

#endif /* INTERFACE_ICLASSNAME_H_ */
