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

#ifndef _SYS_MEMORY_H
#define _SYS_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif


#define PROT_READ          1UL
#define PROT_WRITE         2UL
#define PROT_READ_WRITE    3UL
#define PROT_EXECUTE       4UL
#define PROT_READ_EXECUTE  5UL


#ifndef _IN_KERNEL

#include "syscall.h"

static inline void* mmap(unsigned long size, unsigned long perms, void* addr) {
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_MMAP), "D"(size), "S"(perms), "d"(addr) : "rcx", "r11", "memory");
    return (void*)ret;
}

static inline int munmap(void* addr, unsigned long size) {
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_MUNMAP), "D"(addr), "S"(size) : "rcx", "r11", "memory");
    return (int)ret;
}

static inline int mprotect(void* addr, unsigned long size, unsigned long perms) {
    unsigned long ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(SC_MPROTECT), "D"(addr), "S"(size), "d"(perms) : "rcx", "r11", "memory");
    return (int)ret;
}

#else /* _IN_KERNEL */

// Request page aligned memory size bytes long, starting at addr (if null, any address) with perms. If returned address is between (void*)-1 and (void*)-100, then an error occurred and the address is the error code.
void* mmap(unsigned long size, unsigned long perms, void* addr);

// Unmap page aligned memory size bytes long, starting at addr.
int munmap(void* addr, unsigned long size);

// Remap page aligned memory size bytes long, starting at addr with perms.
int mprotect(void* addr, unsigned long size, unsigned long perms);

#endif /* _IN_KERNEL */

#ifdef __cplusplus
}
#endif

#endif /* _SYS_MEMORY_H */