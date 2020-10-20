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

#ifndef INTERFACE_ILISTENER_H_
#define INTERFACE_ILISTENER_H_

#include <iostream>

using namespace std;

template <class T>
class IListener {
public:
    IListener() : m_listener(nullptr) {};
    virtual ~IListener() {};

    void setListener(T* listener)
    {
        m_listener = listener;
    }

    T* getListener()
    {
        return m_listener;
    }

protected:
    T* m_listener;

};

#endif /* INTERFACE_ILISTENER_H_ */
