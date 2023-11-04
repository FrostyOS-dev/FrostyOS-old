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

#ifndef _SYS_FILE_H
#define _SYS_FILE_H

typedef long fd_t;

#define O_READ 1UL
#define O_WRITE 2UL
#define O_CREATE 4UL
#define O_APPEND 8UL

#ifdef __cplusplus
extern "C" {
#endif


#ifndef _IN_KERNEL

#include "syscall.h"

static inline fd_t open(const char* path, unsigned long mode) {
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_OPEN), "D"(path), "S"(mode) : "rcx", "r11", "memory");
    return (fd_t)ret;
}

static inline long read(fd_t file, void* buf, unsigned long count) {
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_READ), "D"(file), "S"(buf), "d"(count) : "rcx", "r11", "memory");
    return (long)ret;
}

static inline long write(fd_t file, const void* buf, unsigned long count) {
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_WRITE), "D"(file), "S"(buf), "d"(count) : "rcx", "r11", "memory");
    return (long)ret;
}

static inline int close(fd_t file) {
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_CLOSE), "D"(file) : "rcx", "r11", "memory");
    return (int)ret;
}

static inline long seek(fd_t file, long offset) {
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_SEEK), "D"(file), "S"(offset) : "rcx", "r11", "memory");
    return (long)ret;
}

#else /* _IN_KERNEL */

fd_t open(const char* path, unsigned long mode);

long read(fd_t file, void* buf, unsigned long count);
long write(fd_t file, const void* buf, unsigned long count);

int close(fd_t file);

long seek(fd_t file, long offset);

#endif /* _IN_KERNEL */


#ifdef __cplusplus
}
#endif

#endif /* _SYS_FILE_H */