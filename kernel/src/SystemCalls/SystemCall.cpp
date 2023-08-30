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

#include "SystemCall.hpp"

#include <stdio.h>

extern "C" {
    const uint64_t g_syscall_count = SYSTEM_CALL_COUNT;
    SystemCallHandler_t g_syscall_handlers[SYSTEM_CALL_COUNT];
}

uint64_t SystemCallHandler(uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    uint64_t number;
    __asm__ volatile ("movq %%rax, %0" : "=g"(number) :: "rax");
    fprintf(VFS_DEBUG, "Received system call. number = %lu, arg1 = %lx, arg2 = %lx, arg3 = %lx.\n", number, arg1, arg2, arg3);
    return 0;
}

void SystemCallInit() {
    for (uint64_t i = 0; i < g_syscall_count; i++) {
        g_syscall_handlers[i] = SystemCallHandler;
    }
}
