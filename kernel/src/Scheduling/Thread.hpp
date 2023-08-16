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
        void SetStack(uint64_t stack);

        ThreadEntry_t GetEntry() const;
        void* GetEntryData() const;
        uint8_t GetFlags() const;
        Process* GetParent() const;
        CPU_Registers* GetCPURegisters() const;
        uint64_t GetStack() const;

        void Start();

    private:
        Process* m_Parent;
        ThreadEntry_t m_entry;
        void* m_entry_data;
        uint8_t m_flags;
        uint64_t m_stack;
        mutable CPU_Registers m_regs;
    };
}

#endif /* _KERNEL_THREAD_HPP */