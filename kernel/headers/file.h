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

#define EOF -1

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define DT_FILE 0
#define DT_DIR 1
#define DT_SYMLNK 2

struct stat_buf {
    unsigned long st_size;
    unsigned int st_uid;
    unsigned int st_gid;
    unsigned short st_mode;
    unsigned long st_type;
};

struct dirent {
    unsigned long d_type;
    unsigned int d_uid;
    unsigned int d_gid;
    unsigned short d_mode;
    unsigned long d_size;
    char d_name[256];
};

#ifdef __cplusplus
extern "C" {
#endif


#ifndef _IN_KERNEL

#include "syscall.h"

static inline fd_t open(const char* path, unsigned long flags, unsigned short mode) {
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_OPEN), "D"(path), "S"(flags), "d"(mode) : "rcx", "r11", "memory");
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

static inline long seek(fd_t file, long offset, long whence) {
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_SEEK), "D"(file), "S"(offset), "d"(whence) : "rcx", "r11", "memory");
    return (long)ret;
}

static inline unsigned int getuid() {
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_GETUID) : "rcx", "r11", "memory");
    return (unsigned int)ret;
}

static inline unsigned int getgid() {
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_GETGID) : "rcx", "r11", "memory");
    return (unsigned int)ret;
}

static inline unsigned int geteuid() {
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_GETEUID) : "rcx", "r11", "memory");
    return (unsigned int)ret;
}

static inline unsigned int getegid() {
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_GETEGID) : "rcx", "r11", "memory");
    return (unsigned int)ret;
}

static inline int stat(const char* path, struct stat_buf* buf) {
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_STAT), "D"(path), "S"(buf) : "rcx", "r11", "memory");
    return (int)ret;
}

static inline int fstat(fd_t file, struct stat_buf* buf) {
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_FSTAT), "D"(file), "S"(buf) : "rcx", "r11", "memory");
    return (int)ret;
}

static inline int chown(const char* path, unsigned int uid, unsigned int gid) {
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_CHOWN), "D"(path), "S"(uid), "d"(gid) : "rcx", "r11", "memory");
    return (int)ret;
}

static inline int fchown(fd_t file, unsigned int uid, unsigned int gid) {
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_FCHOWN), "D"(file), "S"(uid), "d"(gid) : "rcx", "r11", "memory");
    return (int)ret;
}

static inline int chmod(const char* path, unsigned short mode) {
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_CHMOD), "D"(path), "S"(mode) : "rcx", "r11", "memory");
    return (int)ret;
}

static inline int fchmod(fd_t file, unsigned short mode) {
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_FCHMOD), "D"(file), "S"(mode) : "rcx", "r11", "memory");
    return (int)ret;
}

static inline int getdirents(fd_t file, struct dirent* buf, unsigned long count) {
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_GETDIRENTS), "D"(file), "S"(buf), "d"(count) : "rcx", "r11", "memory");
    return (int)ret;
}

static inline int chdir(const char* path) {
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_CHDIR), "D"(path) : "rcx", "r11", "memory");
    return (int)ret;
}

static inline int fchdir(fd_t file) {
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_FCHDIR), "D"(file) : "rcx", "r11", "memory");
    return (int)ret;
}

static inline int mount(const char* path, const char* type, const char* device) {
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_MOUNT), "D"(path), "S"(type), "d"(device) : "rcx", "r11", "memory");
    return (int)ret;
}

static inline int unmount(const char* path) {
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_UNMOUNT), "D"(path) : "rcx", "r11", "memory");
    return (int)ret;
}

#else /* _IN_KERNEL */

fd_t open(const char* path, unsigned long flags, unsigned short mode);

long read(fd_t file, void* buf, unsigned long count);
long write(fd_t file, const void* buf, unsigned long count);

int close(fd_t file);

long seek(fd_t file, long offset, long whence);

unsigned int getuid();
unsigned int getgid();
unsigned int geteuid();
unsigned int getegid();

int stat(const char* path, struct stat_buf* buf);
int fstat(fd_t file, struct stat_buf* buf);

int chown(const char* path, unsigned int uid, unsigned int gid);
int fchown(fd_t file, unsigned int uid, unsigned int gid);

int chmod(const char* path, unsigned short mode);
int fchmod(fd_t file, unsigned short mode);

int getdirents(fd_t file, struct dirent* buf, unsigned long count);

int chdir(const char* path);
int fchdir(fd_t file);

int mount(const char* path, const char* type, const char* device);
int unmount(const char* path);

#endif /* _IN_KERNEL */


#ifdef __cplusplus
}
#endif

#endif /* _SYS_FILE_H */
