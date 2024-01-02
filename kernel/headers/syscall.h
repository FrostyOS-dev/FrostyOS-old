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

#ifndef _SYSCALL_H
#define _SYSCALL_H

#ifdef __cplusplus
extern "C" {
#endif

enum SystemCalls {
    SC_EXIT = 0,
    SC_READ = 1,
    SC_WRITE = 2,
    SC_OPEN = 3,
    SC_CLOSE = 4,
    SC_SEEK = 5,
    SC_MMAP = 6,
    SC_MUNMAP = 7,
    SC_MPROTECT = 8,
    SC_GETUID = 9,
    SC_GETGID = 10,
    SC_GETEUID = 11,
    SC_GETEGID = 12,
    SC_STAT = 13,
    SC_FSTAT = 14,
    SC_CHOWN = 15,
    SC_FCHOWN = 16,
    SC_CHMOD = 17,
    SC_FCHMOD = 18,
    SC_GETPID = 19,
    SC_GETTID = 20,
    SC_EXEC = 21,
    SC_SLEEP = 22,
    SC_MSLEEP = 23,
    SC_GETDIRENTS = 24,
    SC_CHDIR = 25,
    SC_FCHDIR = 26,
    SC_ONSIGNAL = 27,
    SC_SENDSIG = 28,
    SC_SIGRETURN = 29
};

#ifndef _IN_KERNEL

inline unsigned long system_call(unsigned long num, unsigned long arg1, unsigned long arg2, unsigned arg3) {
    // Number is in RAX, arg1 is in RDI, arg2 is in RSI, arg3 is in RDX
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3) : "rcx", "r11", "memory");
    return ret;
}

#endif /* _IN_KERNEL */


#ifdef __cplusplus
}
#endif

#endif /* _SYS_SYSCALL_H */