/*
Copyright (©) 2023  Frosty515

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

typedef unsigned long pid_t;
typedef unsigned long tid_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _IN_KERNEL

#include "syscall.h"

pid_t getpid() {
    pid_t ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_GETPID) : "rcx", "r11", "memory");
    return ret;
}

tid_t gettid() {
    tid_t ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_GETTID) : "rcx", "r11", "memory");
    return ret;
}

int exec(const char *path, char *const argv[], char *const envv[]) {
    int ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_EXEC), "D"(path), "S"(argv), "d"(envv) : "rcx", "r11", "memory");
    return ret;
}

#else

pid_t getpid();

tid_t gettid();

int exec(const char *path, char *const argv[], char *const envv[]);

#endif /* _IN_KERNEL */

#ifdef __cplusplus
}
#endif

#endif /* _PROCESS_H */