#include "Thread.hpp"

#include "Scheduler.hpp"

namespace Scheduling {

    Thread::Thread(Process* parent, ThreadEntry_t entry, void* entry_data, uint8_t flags) : m_Parent(parent), m_entry(entry), m_entry_data(entry_data), m_flags(flags), m_stack(0) {
        fast_memset(&m_regs, 0, DIV_ROUNDUP(sizeof(m_regs), 8));
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
        return &m_regs;
    }

    uint64_t Thread::GetStack() const {
        return m_stack;
    }

    void Thread::Start() {
        Scheduler::ScheduleThread(this);
    }

}