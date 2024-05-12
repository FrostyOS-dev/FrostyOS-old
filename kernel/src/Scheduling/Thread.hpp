/*
Copyright (©) 2022-2024  Frosty515

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

#ifndef _THREAD_HPP
#define _THREAD_HPP

#include "Process.hpp"

#include <stdint.h>

#include <fs/FileDescriptorManager.hpp>
#include <fs/VFS.hpp>

#include <file.h>

namespace Scheduling {

    class Semaphore;

    typedef void (*ThreadEntry_t)(void*);
    struct ThreadCleanup_t {
        void (*function)(void*);
        void* data;
    };

    constexpr uint8_t THREAD_KERNEL_DEFAULT = CREATE_STACK;
    constexpr uint8_t THREAD_USER_DEFAULT = CREATE_STACK;

    class Thread {
    public:
        struct Register_Frame {
            uint64_t user_stack;
            uint64_t kernel_stack;
        } __attribute__((packed));

        Thread(Process* parent, ThreadEntry_t entry = nullptr, void* entry_data = nullptr, uint8_t flags = THREAD_USER_DEFAULT, tid_t TID = -1);
        ~Thread();

        void SetEntry(ThreadEntry_t entry, void* entry_data);
        void SetFlags(uint8_t flags);
        void SetParent(Process* parent);
        void SetStack(uint64_t stack);
        void SetCleanupFunction(ThreadCleanup_t cleanup);

        ThreadEntry_t GetEntry() const;
        void* GetEntryData() const;
        uint8_t GetFlags() const;
        Process* GetParent() const;
        CPU_Registers* GetCPURegisters() const;
        uint64_t GetStack() const;
        uint64_t GetKernelStack() const;
        ThreadCleanup_t GetCleanupFunction() const;
        Register_Frame* GetStackRegisterFrame() const;

        void Start();

        fd_t sys_open(const char* path, unsigned long flags, unsigned short mode);
        long sys_read(fd_t file, void* buf, unsigned long count);
        long sys_write(fd_t file, const void* buf, unsigned long count);
        int sys_close(fd_t file);
        long sys_seek(fd_t file, long offset, long whence);

        int sys_stat(const char* path, struct stat_buf* buf);
        int sys_fstat(fd_t file, struct stat_buf* buf);
        int sys_chown(const char* path, unsigned int uid, unsigned int gid);
        int sys_fchown(fd_t file, unsigned int uid, unsigned int gid);
        int sys_chmod(const char* path, unsigned short mode);
        int sys_fchmod(fd_t file, unsigned short mode);

        void sys_sleep(unsigned long s);
        void sys_msleep(unsigned long ms);

        int sys_getdirents(fd_t file, struct dirent* dirp, unsigned long count);

        int sys_chdir(const char* path);
        int sys_fchdir(fd_t file);

        void PrintInfo(fd_t file) const;

        void SetTID(tid_t TID);
        tid_t GetTID() const;

        bool IsSleeping() const;
        void SetSleeping(bool sleeping);

        uint64_t GetRemainingSleepTime() const;
        void SetRemainingSleepTime(uint64_t time);

        bool IsIdle() const;
        void SetIdle(bool idle);

        bool IsBlocked() const;
        void SetBlocked(bool blocked);

        VFS_WorkingDirectory* GetWorkingDirectory() const;
        void SetWorkingDirectory(VFS_WorkingDirectory* working_directory);

        Thread* GetNextThread();
        Thread* GetPreviousThread();
        void SetNextThread(Thread* next_thread);
        void SetPreviousThread(Thread* previous_thread);

    private:
        Process* m_Parent;
        ThreadEntry_t m_entry;
        void* m_entry_data;
        uint8_t m_flags;
        uint64_t m_stack;
        mutable Register_Frame m_frame;
        mutable CPU_Registers m_regs;
        ThreadCleanup_t m_cleanup;
        FileDescriptorManager m_FDManager;

        tid_t m_TID;

        bool m_sleeping;
        uint64_t m_remaining_sleep_time;

        bool m_idle;

        bool m_blocked;

        VFS_WorkingDirectory* m_working_directory;

        Thread* m_next_thread;
        Thread* m_previous_thread;
    };
}

#endif /* _KERNEL_THREAD_HPP */