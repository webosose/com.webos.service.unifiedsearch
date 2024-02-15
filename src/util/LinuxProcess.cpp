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

#include <signal.h>
#include <glib.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <array>

#include "util/LinuxProcess.h"

#if HAS_LIBPROC2
#include <libproc2/pids.h>
#else
#include <proc/readproc.h>
#endif

const string LinuxProcess::CLASS_NAME = "LinuxProcess";

bool LinuxProcess::sendSigTerm(const string& pid)
{
    if (pid.empty()) {
        Logger::error(CLASS_NAME, __FUNCTION__, "pid is empty");
        return false;
    }

    PidVector allPids = LinuxProcess::findChildPids(pid);
    if (allPids.empty()) {
        Logger::error(CLASS_NAME, __FUNCTION__, "empty_pids");
        return false;
    }

    if (!LinuxProcess::killProcesses(allPids, SIGTERM)) {
        Logger::error(CLASS_NAME, __FUNCTION__, "seding_signal_error");
        return false;
    }
    return true;
}

bool LinuxProcess::sendSigKill(const string& pid)
{
    if (pid.empty()) {
        Logger::error(CLASS_NAME, __FUNCTION__, "pid is empty");
        return false;
    }

    PidVector allPids = LinuxProcess::findChildPids(pid);
    if (allPids.empty()) {
        Logger::error(CLASS_NAME, __FUNCTION__, "empty_pids");
        return false;
    }

    if (!LinuxProcess::killProcesses(allPids, SIGKILL)) {
        Logger::error(CLASS_NAME, __FUNCTION__, "seding_signal_error");
        return false;
    }
    return true;
}

string LinuxProcess::convertPidsToString(const PidVector& pids)
{
    string result;
    string delim;
    for (pid_t pid : pids) {
        result += delim;
        result += to_string(pid);
        delim = " ";
    }
    return result;
}

bool LinuxProcess::killProcesses(const PidVector& pids, int sig)
{
    auto it = pids.begin();
    if (it == pids.end())
        return true;

    // first process is parent process,
    // killing child processes later can fail if parent itself terminates them
    bool success = kill(*it, sig) == 0;
    while (++it != pids.end()) {
        kill(*it, sig);
    }
    return success;
}

bool LinuxProcess::forkSyncProcess(const char **argv, const char **envp, int *exit_status)
{
    GError* gerr = NULL;
    GSpawnFlags flags = (GSpawnFlags) (G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL | G_SPAWN_CHILD_INHERITS_STDIN);

    if (argv) {
        string cmd = "";
        for (char const **p = argv; *p; p++) {
            cmd += string(*p) + " ";
        }
        Logger::info(CLASS_NAME, __FUNCTION__, cmd);
    }

    gboolean result = g_spawn_sync(
        NULL,
        const_cast<char**>(argv),  // cmd arguments
        const_cast<char**>(envp),  // environment variables
        flags,
        NULL,
        NULL,
        NULL,
        NULL,
        exit_status,
        &gerr
    );
    if (gerr) {
        Logger::error(CLASS_NAME, __FUNCTION__, Logger::format("Failed to folk: errorText(%s)", gerr->message));
        g_error_free(gerr);
        gerr = NULL;
        return false;
    }
    if (!result) {
        Logger::error(CLASS_NAME, __FUNCTION__, "Failed to fork");
        return false;
    }

    return true;
}

pid_t LinuxProcess::forkAsyncProcess(const char **argv, const char **envp)
{
    GPid pid = -1;
    GError* gerr = NULL;
    GSpawnFlags flags = (GSpawnFlags) (G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL | G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_CHILD_INHERITS_STDIN);

    if (argv) {
        string cmd = "";
        for (char const **p = argv; *p; p++) {
            cmd += string(*p) + " ";
        }
        Logger::info(CLASS_NAME, __FUNCTION__, cmd);
    }

    gboolean result = g_spawn_async_with_pipes(
        NULL,
        const_cast<char**>(argv),  // cmd arguments
        const_cast<char**>(envp),  // environment variables
        flags,
        NULL,
        NULL,
        &pid,
        NULL,
        NULL,
        NULL,
        &gerr
    );
    if (gerr) {
        Logger::error(CLASS_NAME, __FUNCTION__, Logger::format("Failed to folk: pid(%d) errorText(%s)", pid, gerr->message));
        g_error_free(gerr);
        gerr = NULL;
        return -1;
    }
    if (!result) {
        Logger::error(CLASS_NAME, __FUNCTION__, Logger::format("Failed to folk: pid: %d", pid));
        return -1;
    }

    return pid;
}

PidVector LinuxProcess::findChildPids(const string& pid)
{
    PidVector pids;
    pids.push_back((pid_t) atol(pid.c_str()));

#if HAS_LIBPROC2
#warning TODO migrate to proc2
#else
    proc_t **proctab = readproctab(PROC_FILLSTAT);
    if (!proctab) {
        Logger::error(CLASS_NAME, __FUNCTION__, "readproctab_error", "failed to read proctab");
        return pids;
    }

    size_t idx = 0;
    while (idx != pids.size()) {
        for (proc_t **proc = proctab; *proc; ++proc) {
            pid_t tid = (*proc)->tid;
            pid_t ppid = (*proc)->ppid;
            if (ppid == pids[idx]) {
                pids.push_back(tid);
            }
        }
        ++idx;
    }

    for (proc_t **proc = proctab; *proc; ++proc) {
        free(*proc);
    }

    free(proctab);
#endif
    return pids;
}

string LinuxProcess::getStdoutFromCmd(const string& cmd)
{
    array<char, 128> buffer;
    string result = "";
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        return "";
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

int LinuxProcess::getUid(const string& userName)
{
    struct passwd *user;
    user = getpwnam(userName.c_str());
    if (user == NULL) {
        return -1;
    }
    return user->pw_uid;
}
