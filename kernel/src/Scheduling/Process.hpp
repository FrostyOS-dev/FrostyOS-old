#ifndef _KERNEL_PROCESS_HPP
#define _KERNEL_PROCESS_HPP

#include <stdint.h>

#include <HAL/hal.hpp>

#include <Data-structures/LinkedList.hpp>

#include <Memory/PageManager.hpp>

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
        Process(ProcessEntry_t entry, void* entry_data = nullptr, Priority priority = Priority::NORMAL, uint8_t flags = USER_DEFAULT, WorldOS::PageManager* pm = nullptr);
        ~Process();

        void SetEntry(ProcessEntry_t entry, void* entry_data = nullptr);
        void SetPriority(Priority priority);
        void SetFlags(uint8_t flags);
        void SetPageManager(WorldOS::PageManager* pm);

        ProcessEntry_t GetEntry() const;
        void* GetEntryData() const;
        Priority GetPriority() const;
        uint8_t GetFlags() const;
        WorldOS::PageManager* GetPageManager() const;

        void Start();
        void ScheduleThread(Thread* thread);

    private:
        ProcessEntry_t m_Entry;
        void* m_entry_data;
        uint8_t m_flags;
        Priority m_Priority;
        WorldOS::PageManager* m_pm;
        bool m_main_thread_initialised;
        LinkedList::SimpleLinkedList<Thread> m_threads;
        Thread* m_main_thread;
    };
}

#endif /* _KERNEL_PROCESS_HPP */