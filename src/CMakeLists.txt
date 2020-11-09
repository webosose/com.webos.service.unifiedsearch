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

pkg_check_modules(PROCPS REQUIRED libprocps)
include_directories(PROCPS_INCLUDE_DIRS)
webos_add_compiler_flags(ALL ${PROCPS_CFLAGS})

pkg_check_modules(SQLITE3 REQUIRED sqlite3)
include_directories(SQLITE3_INCLUDE_DIRS)
webos_add_compiler_flags(ALL ${SQLITE3_CFLAGS})

# Environment
webos_configure_source_files(cfg ${CMAKE_CURRENT_SOURCE_DIR}/Environment.h)
webos_add_compiler_flags(ALL CXX -std=c++0x)

set(BIN_NAME unifiedsearch)
file(GLOB_RECURSE SRC_CPP ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(../core)
include_directories(${PROJECT_BINARY_DIR}/Configured/src)
add_executable(${BIN_NAME} ${SRC_CPP})

set(LIBS
    ${GLIB2_LDFLAGS}
    ${PROCPS_LDFLAGS}
    ${SQLITE3_LDFLAGS}
    -pthread
    -ldl
)
target_link_libraries(${BIN_NAME} searchcore ${LIBS})

# Install
install(TARGETS ${BIN_NAME} DESTINATION ${WEBOS_INSTALL_SBINDIR})

webos_build_system_bus_files()

webos_build_daemon(NAME ${BIN_NAME})
webos_build_configured_file(files/conf/unifiedsearch.json WEBOS_SYSCONFDIR "")