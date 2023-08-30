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
