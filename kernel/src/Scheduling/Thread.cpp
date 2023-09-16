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

#include "Thread.hpp"

#include "Scheduler.hpp"

namespace Scheduling {

    Thread::Thread(Process* parent, ThreadEntry_t entry, void* entry_data, uint8_t flags, uint64_t kernel_stack) : m_Parent(parent), m_entry(entry), m_entry_data(entry_data), m_flags(flags), m_stack(0), m_cleanup({nullptr, nullptr}) {
        fast_memset(&m_regs, 0, DIV_ROUNDUP(sizeof(m_regs), 8));
        m_regs.kernel_stack = kernel_stack;
    }

    Thread::~Thread() {

    }

    void Thread::SetEntry(ThreadEntry_t entry, void* entry_data) {
        m_entry = entry;
        m_entry_data = entry_data;
    }

    void Thread::SetFlags(uint8_t flags) {
        m_flags = flags;
    }

    void Thread::SetParent(Process* parent) {
        m_Parent = parent;
    }

    void Thread::SetStack(uint64_t stack) {
        m_stack = stack;
        if (m_Parent != nullptr) {
            if (m_Parent->GetPriority() == Priority::KERNEL)
                m_regs.kernel_stack = stack;
        }
    }

    void Thread::SetKernelStack(uint64_t kernel_stack) {
        m_regs.kernel_stack = kernel_stack;
    }

    void Thread::SetCleanupFunction(ThreadCleanup_t cleanup) {
        m_cleanup = cleanup;
    }

    ThreadEntry_t Thread::GetEntry() const {
        return m_entry;
    }

    void* Thread::GetEntryData() const {
        return m_entry_data;
    }

    uint8_t Thread::GetFlags() const {
        return m_flags;
    }

    Process* Thread::GetParent() const {
        return m_Parent;
    }

    CPU_Registers* Thread::GetCPURegisters() const {
        return &m_regs.regs;
    }

    uint64_t Thread::GetStack() const {
        return m_stack;
    }

    uint64_t Thread::GetKernelStack() const {
        return m_regs.kernel_stack;
    }

    ThreadCleanup_t Thread::GetCleanupFunction() const {
        return m_cleanup;
    }

    void Thread::Start() {
        Scheduler::ScheduleThread(this);
    }

}