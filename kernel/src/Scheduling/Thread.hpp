#ifndef _KERNEL_THREAD_HPP
#define _KERNEL_THREAD_HPP

#include "Process.hpp"

#include <stdint.h>

namespace Scheduling {

    typedef void (*ThreadEntry_t)(void*);

    constexpr uint8_t THREAD_KERNEL_DEFAULT = CREATE_STACK;
    constexpr uint8_t THREAD_USER_DEFAULT = CREATE_STACK;

    class Thread {
    public:
        Thread(Process* parent, ThreadEntry_t entry = nullptr, void* entry_data = nullptr, uint8_t flags = THREAD_USER_DEFAULT);
        ~Thread();

        void SetEntry(ThreadEntry_t entry, void* entry_data);
        void SetFlags(uint8_t flags);
        void SetParent(Process* parent);
        void UpdateCPURegisters(CPU_Registers* regs);

        ThreadEntry_t GetEntry() const;
        void* GetEntryData() const;
        uint8_t GetFlags() const;
        Process* GetParent() const;
        CPU_Registers* GetCPURegisters() const;

        void Start();

    private:
        Process* m_Parent;
        ThreadEntry_t m_entry;
        void* m_entry_data;
        uint8_t m_flags;
        CPU_Registers* m_regs;
    };
}

#endif /* _KERNEL_THREAD_HPP */