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
#include "exec.hpp"

#include <stdio.h>
#include <errno.h>

#include <Scheduling/Scheduler.hpp>
#include <Scheduling/Thread.hpp>

#include <syscall.h>

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
        return (uint64_t)(current_thread->sys$open((const char*)arg1, arg2, (unsigned short)arg3));
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
    case SC_GETUID: {
        Scheduling::Process* parent = current_thread->GetParent();
        if (parent == nullptr)
            return (uint64_t)-EFAULT;
        return (uint64_t)(parent->GetUID());
    }
    case SC_GETGID: {
        Scheduling::Process* parent = current_thread->GetParent();
        if (parent == nullptr)
            return (uint64_t)-EFAULT;
        return (uint64_t)(parent->GetGID());
    }
    case SC_GETEUID: {
        Scheduling::Process* parent = current_thread->GetParent();
        if (parent == nullptr)
            return (uint64_t)-EFAULT;
        return (uint64_t)(parent->GetEUID());
    }
    case SC_GETEGID: {
        Scheduling::Process* parent = current_thread->GetParent();
        if (parent == nullptr)
            return (uint64_t)-EFAULT;
        return (uint64_t)(parent->GetEGID());
    }
    case SC_STAT:
        return (uint64_t)(current_thread->sys$stat((const char*)arg1, (struct stat_buf*)arg2));
    case SC_FSTAT:
        return (uint64_t)(current_thread->sys$fstat((fd_t)arg1, (struct stat_buf*)arg2));
    case SC_CHOWN:
        return (uint64_t)(current_thread->sys$chown((const char*)arg1, (unsigned int)arg2, (unsigned int)arg3));
    case SC_FCHOWN:
        return (uint64_t)(current_thread->sys$fchown((fd_t)arg1, (unsigned int)arg2, (unsigned int)arg3));
    case SC_CHMOD:
        return (uint64_t)(current_thread->sys$chmod((const char*)arg1, (unsigned short)arg2));
    case SC_FCHMOD:
        return (uint64_t)(current_thread->sys$fchmod((fd_t)arg1, (unsigned short)arg2));
    case SC_GETPID: {
        Scheduling::Process* parent = current_thread->GetParent();
        if (parent == nullptr)
            return (uint64_t)-EFAULT;
        return (uint64_t)(parent->GetPID());
    }
    case SC_GETTID:
        return (uint64_t)(current_thread->GetTID());
    case SC_EXEC:
        return sys$exec(current_thread->GetParent(), (const char*)arg1, (char* const*)arg2, (char* const*)arg3);
    case SC_SLEEP:
        current_thread->sys$sleep(arg1);
        return 0;
    case SC_MSLEEP:
        current_thread->sys$msleep(arg1);
        return 0;
    default:
        dbgprintf("Unknown system call. number = %lu, arg1 = %lx, arg2 = %lx, arg3 = %lx.\n", num, arg1, arg2, arg3);
        return -1;
    }
}

void SystemCallInit() {
    
}
