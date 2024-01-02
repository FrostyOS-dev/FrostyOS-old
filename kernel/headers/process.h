/*
Copyright (©) 2023-2024  Frosty515

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _PROCESS_H
#define _PROCESS_H

typedef long int pid_t;
typedef long int tid_t;

#ifdef __cplusplus
extern "C" {
#endif

#define SIGABRT 1
#define SIGFPE 2
#define SIGILL 3
#define SIGINT 4
#define SIGSEGV 5
#define SIGTERM 6
#define SIGKILL 7
#define SIGCHLD 8

#define SIG_DFL 1
#define SIG_IGN 2
#define SIG_ERR -1

// The offset that is added to the signal number when the process is killed on a signal.
#define SIG_RET_STATUS_OFFSET 128

#define SIG_COUNT 8

#define SIG_MAX 8
#define SIG_MIN 1

struct signal_action {
    void (*handler)(int);
    int flags;
};

#ifndef _IN_KERNEL

#include "syscall.h"

static inline pid_t getpid() {
    pid_t ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_GETPID) : "rcx", "r11", "memory");
    return ret;
}

static inline tid_t gettid() {
    tid_t ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_GETTID) : "rcx", "r11", "memory");
    return ret;
}

// Execute a new process, return the pid of the new process on success.
static inline pid_t exec(const char *path, char *const argv[], char *const envv[]) {
    pid_t ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_EXEC), "D"(path), "S"(argv), "d"(envv) : "rcx", "r11", "memory");
    return ret;
}

// If new_action is non-NULL, set signal action to new_action. If old_action is non-NULL, set old_action to the old signal action.
static inline int onsignal(int signum, const struct signal_action* new_action, struct signal_action* old_action) {
    int ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_ONSIGNAL), "D"(signum), "S"(new_action), "d"(old_action) : "rcx", "r11", "memory");
    return ret;
}

// Send a signal to a process.
static inline int sendsig(pid_t pid, int signum) {
    int ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_SENDSIG), "D"(pid), "S"(signum) : "rcx", "r11", "memory");
    return ret;
}

#else

pid_t getpid();

tid_t gettid();

// Execute a new process, return the pid of the new process on success.
pid_t exec(const char *path, char *const argv[], char *const envv[]);

// If new_action is non-NULL, set signal action to new_action. If old_action is non-NULL, set old_action to the old signal action.
int onsignal(int signum, const struct signal_action* new_action, struct signal_action* old_action);

// Send a signal to a process.
int sendsig(pid_t pid, int signum);

#endif /* _IN_KERNEL */

#ifdef __cplusplus
}
#endif

#endif /* _PROCESS_H */