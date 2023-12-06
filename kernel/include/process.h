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