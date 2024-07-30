# @@@LICENSE
#
# Copyright (c) 2020 LG Electronics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# LICENSE@@@

# Find Packages
include(FindPkgConfig)

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${GLIB2_CFLAGS_OTHER})

pkg_check_modules(PBNJSON_C REQUIRED pbnjson_c)
include_directories(${PBNJSON_C_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${PBNJSON_C_CFLAGS_OTHER})

pkg_check_modules(PBNJSON_CPP REQUIRED pbnjson_cpp)
include_directories(${PBNJSON_CPP_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${PBNJSON_CPP_CFLAGS_OTHER})

pkg_check_modules(LUNASERVICE2 luna-service2)
include_directories(${LUNASERVICE2_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${LUNASERVICE2_CFLAGS_OTHER})

pkg_check_modules(LUNASERVICE2CPP luna-service2++)
include_directories(${LUNASERVICE2CPP_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${LUNASERVICE2CPP_CFLAGS_OTHER})

# Environment

set(LIB_NAME searchcore)
file(GLOB_RECURSE SRC_H ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE SRC_CPP ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(${LIB_NAME} SHARED ${SRC_CPP})

target_link_libraries(
    ${LIB_NAME}
    ${GLIB2_LDFLAGS}
    ${LUNASERVICE2_LDFLAGS}
    ${LUNASERVICE2CPP_LDFLAGS}
    ${PBNJSON_C_LDFLAGS}
    ${PBNJSON_CPP_LDFLAGS}
)

set_target_properties(${LIB_NAME} PROPERTIES VERSION ${WEBOS_API_VERSION} SOVERSION ${WEBOS_API_VERSION_MAJOR})

# Install
install(FILES ${SRC_H} DESTINATION ${WEBOS_INSTALL_INCLUDEDIR}/searchcore)
install(TARGETS ${LIB_NAME} DESTINATION ${WEBOS_INSTALL_LIBDIR})
