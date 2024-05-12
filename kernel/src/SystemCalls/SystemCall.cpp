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

#include "SystemCall.hpp"
#include "exit.hpp"
#include "memory.hpp"
#include "exec.hpp"
#include "mount.hpp"
#include "Synchronisation.hpp"

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
        sys_exit(current_thread, (int)arg1);
        return 0;
    case SC_READ:
        return (uint64_t)(current_thread->sys_read((fd_t)arg1, (void*)arg2, arg3));
    case SC_WRITE:
        return (uint64_t)(current_thread->sys_write((fd_t)arg1, (const void*)arg2, arg3));
    case SC_OPEN:
        return (uint64_t)(current_thread->sys_open((const char*)arg1, arg2, (unsigned short)arg3));
    case SC_CLOSE:
        return (uint64_t)(current_thread->sys_close((fd_t)arg1));
    case SC_SEEK:
        return (uint64_t)(current_thread->sys_seek((fd_t)arg1, (long)arg2, (long)arg3));
    case SC_MMAP:
        return (uint64_t)sys_mmap(arg1, arg2, (void*)arg3);
    case SC_MUNMAP:
        return (uint64_t)sys_munmap((void*)arg1, arg2);
    case SC_MPROTECT:
        return (uint64_t)sys_mprotect((void*)arg1, arg2, arg3);
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
        return (uint64_t)(current_thread->sys_stat((const char*)arg1, (struct stat_buf*)arg2));
    case SC_FSTAT:
        return (uint64_t)(current_thread->sys_fstat((fd_t)arg1, (struct stat_buf*)arg2));
    case SC_CHOWN:
        return (uint64_t)(current_thread->sys_chown((const char*)arg1, (unsigned int)arg2, (unsigned int)arg3));
    case SC_FCHOWN:
        return (uint64_t)(current_thread->sys_fchown((fd_t)arg1, (unsigned int)arg2, (unsigned int)arg3));
    case SC_CHMOD:
        return (uint64_t)(current_thread->sys_chmod((const char*)arg1, (unsigned short)arg2));
    case SC_FCHMOD:
        return (uint64_t)(current_thread->sys_fchmod((fd_t)arg1, (unsigned short)arg2));
    case SC_GETPID: {
        Scheduling::Process* parent = current_thread->GetParent();
        if (parent == nullptr)
            return (uint64_t)-EFAULT;
        return (uint64_t)(parent->GetPID());
    }
    case SC_GETTID:
        return (uint64_t)(current_thread->GetTID());
    case SC_EXEC:
        return sys_exec(current_thread->GetParent(), (const char*)arg1, (char* const*)arg2, (char* const*)arg3);
    case SC_SLEEP:
        current_thread->sys_sleep(arg1);
        return 0;
    case SC_MSLEEP:
        current_thread->sys_msleep(arg1);
        return 0;
    case SC_GETDIRENTS:
        return (uint64_t)(current_thread->sys_getdirents((fd_t)arg1, (struct dirent*)arg2, (size_t)arg3));
    case SC_CHDIR:
        return (uint64_t)(current_thread->sys_chdir((const char*)arg1));
    case SC_FCHDIR:
        return (uint64_t)(current_thread->sys_fchdir((fd_t)arg1));
    case SC_ONSIGNAL: {
        Scheduling::Process* parent = current_thread->GetParent();
        if (parent == nullptr)
            return (uint64_t)-EFAULT;
        return (uint64_t)(parent->sys_onsignal((int)arg1, (const signal_action*)arg2, (signal_action*)arg3));
    }
    case SC_SENDSIG: {
        Scheduling::Process* parent = current_thread->GetParent();
        if (parent == nullptr)
            return (uint64_t)-EFAULT;
        return (uint64_t)(parent->sys_sendsig((pid_t)arg1, (int)arg2));
    }
    case SC_SIGRETURN: {
        Scheduling::Process* parent = current_thread->GetParent();
        if (parent == nullptr) {
            // there is nothing we can do here. We got a noreturn syscall with a parent-less thread.
            PANIC("sigreturn from thread with no parent");
        }
        parent->sys_sigreturn((int)arg1);
        PANIC("sys_sigreturn returned.");
    }
    case SC_MOUNT:
        return (uint64_t)(sys_mount(current_thread, (const char*)arg1, (const char*)arg2, (const char*)arg3));
    case SC_UNMOUNT:
        return (uint64_t)(sys_unmount(current_thread, (const char*)arg1));
    case SC_CREATE_SEMAPHORE:
        return (uint64_t)(sys_createSemaphore((int)arg1));
    case SC_DESTROY_SEMAPHORE:
        return (uint64_t)(sys_destroySemaphore((int)arg1));
    case SC_ACQUIRE_SEMAPHORE:
        return (uint64_t)(sys_acquireSemaphore((int)arg1));
    case SC_RELEASE_SEMAPHORE:
        return (uint64_t)(sys_releaseSemaphore((int)arg1));
    case SC_CREATE_MUTEX:
        return (uint64_t)(sys_createMutex());
    case SC_DESTROY_MUTEX:
        return (uint64_t)(sys_destroyMutex((int)arg1));
    case SC_ACQUIRE_MUTEX:
        return (uint64_t)(sys_acquireMutex((int)arg1));
    case SC_RELEASE_MUTEX:
        return (uint64_t)(sys_releaseMutex((int)arg1));
    default:
        dbgprintf("Unknown system call. number = %lu, arg1 = %lx, arg2 = %lx, arg3 = %lx.\n", num, arg1, arg2, arg3);
        return -1;
    }
}

void SystemCallInit() {
    
}
