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

#ifndef _THREADS_H
#define _THREADS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _IN_KERNEL

#include "syscall.h"

void sleep(unsigned long s) {
    __asm__ volatile("syscall" : : "a"(SC_SLEEP), "D"(s) : "rcx", "r11", "memory");
}

void msleep(unsigned long ms) {
    __asm__ volatile("syscall" : : "a"(SC_MSLEEP), "D"(ms) : "rcx", "r11", "memory");
}

#else

void sleep(unsigned long s);
void msleep(unsigned long ms);

#endif /* _IN_KERNEL */

#ifdef __cplusplus
}
#endif

#endif /* _THREADS_H */