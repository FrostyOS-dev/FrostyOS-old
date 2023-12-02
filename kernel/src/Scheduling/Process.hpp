/*
Copyright (©) 2022-2023  Frosty515

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

#ifndef _KERNEL_PROCESS_HPP
#define _KERNEL_PROCESS_HPP

#include <stdint.h>

#include <HAL/hal.hpp>

#include <Data-structures/LinkedList.hpp>

#include <Memory/PageManager.hpp>
#include <Memory/VirtualRegion.hpp>
#include <Memory/VirtualPageManager.hpp>

namespace Scheduling {
    class Thread;

    typedef void (*ProcessEntry_t)(void*);

    constexpr uint8_t CREATE_STACK = 1;
    constexpr uint8_t ALLOCATE_VIRTUAL_SPACE = 2;
    constexpr uint8_t KERNEL_DEFAULT = CREATE_STACK;
    constexpr uint8_t USER_DEFAULT = CREATE_STACK | ALLOCATE_VIRTUAL_SPACE;

    enum class Priority {
        KERNEL = 0,
        HIGH = 1,
        NORMAL = 2,
        LOW = 3
    };

    class Process {
    public:
        Process();
        Process(ProcessEntry_t entry, void* entry_data, uint32_t UID, uint32_t GID, Priority priority = Priority::NORMAL, uint8_t flags = USER_DEFAULT, PageManager* pm = nullptr);
        ~Process();

        void SetEntry(ProcessEntry_t entry, void* entry_data = nullptr);
        void SetPriority(Priority priority);
        void SetFlags(uint8_t flags);
        void SetPageManager(PageManager* pm);
        void SetRegion(const VirtualRegion& region);
        void SetVirtualPageManager(VirtualPageManager* VPM);

        ProcessEntry_t GetEntry() const;
        void* GetEntryData() const;
        Priority GetPriority() const;
        uint8_t GetFlags() const;
        PageManager* GetPageManager() const;
        const VirtualRegion& GetRegion() const;
        VirtualPageManager* GetVirtualPageManager() const;
        Thread* GetMainThread() const;
        uint64_t GetThreadCount() const;
        Thread* GetThread(uint64_t index) const;

        void CreateMainThread();
        void Start();
        void ScheduleThread(Thread* thread);
        void RemoveThread(Thread* thread);
        void RemoveThread(uint64_t index);

        bool ValidateRead(const void* buf, size_t size) const;
        bool ValidateStringRead(const char* str) const;
        bool ValidateWrite(void* buf, size_t size) const;

        void SyncRegion(); // ensure the region matches the page manager's region

        void SetUID(uint32_t uid);
        void SetGID(uint32_t gid);
        void SetEUID(uint32_t euid);
        void SetEGID(uint32_t egid);

        uint32_t GetUID() const;
        uint32_t GetGID() const;
        uint32_t GetEUID() const;
        uint32_t GetEGID() const;

    private:
        ProcessEntry_t m_Entry;
        void* m_entry_data;
        uint8_t m_flags;
        Priority m_Priority;
        PageManager* m_pm;
        bool m_main_thread_initialised;
        LinkedList::SimpleLinkedList<Thread> m_threads;
        Thread* m_main_thread;
        VirtualRegion m_region;
        VirtualPageManager* m_VPM;
        bool m_main_thread_creation_requested;
        bool m_region_allocated;

        uint32_t m_UID;
        uint32_t m_GID;
        uint32_t m_EUID; // effective UID
        uint32_t m_EGID; // effective GID
    };
}

#endif /* _KERNEL_PROCESS_HPP */