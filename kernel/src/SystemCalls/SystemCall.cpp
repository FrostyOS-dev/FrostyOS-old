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
#include "exit.hpp"
#include "memory.hpp"

#include <stdio.h>

#include <Scheduling/Scheduler.hpp>
#include <Scheduling/Thread.hpp>

#ifdef __x86_64__
#include <arch/x86_64/io.h>
#endif

extern "C" uint64_t SystemCallHandler(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3, CPU_Registers* regs) {
#ifdef __x86_64__
    x86_64_DisableInterrupts();
#endif
    Scheduling::Thread* current_thread = Scheduling::Scheduler::GetCurrent();
    fast_memcpy(current_thread->GetCPURegisters(), regs, sizeof(CPU_Registers)); // save the registers
#ifdef __x86_64__
    x86_64_EnableInterrupts();
#endif
    switch (num) {
    case SC_EXIT:
        sys$exit(current_thread, (int)arg1);
        return 0;
    case SC_READ:
        return (uint64_t)(current_thread->sys$read((fd_t)arg1, (void*)arg2, arg3));
    case SC_WRITE:
        return (uint64_t)(current_thread->sys$write((fd_t)arg1, (const void*)arg2, arg3));
    case SC_OPEN:
        return (uint64_t)(current_thread->sys$open((const char*)arg1, arg2));
    case SC_CLOSE:
        return (uint64_t)(current_thread->sys$close((fd_t)arg1));
    case SC_SEEK:
        return (uint64_t)(current_thread->sys$seek((fd_t)arg1, (long)arg2));
    case SC_MMAP:
        return (uint64_t)sys$mmap(arg1, arg2, (void*)arg3);
    case SC_MUNMAP:
        return (uint64_t)sys$munmap((void*)arg1, arg2);
    case SC_MPROTECT:
        return (uint64_t)sys$mprotect((void*)arg1, arg2, arg3);
    default:
        dbgprintf("Unknown system call. number = %lu, arg1 = %lx, arg2 = %lx, arg3 = %lx.\n", num, arg1, arg2, arg3);
        return -1;
    }
}

void SystemCallInit() {
    
}
