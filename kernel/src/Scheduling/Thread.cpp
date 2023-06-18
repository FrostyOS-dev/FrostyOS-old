#include "Thread.hpp"

#include "Scheduler.hpp"

namespace Scheduling {

    Thread::Thread(Process* parent, ThreadEntry_t entry, void* entry_data, uint8_t flags) : m_Parent(parent), m_entry(entry), m_entry_data(entry_data), m_flags(flags), m_regs(nullptr) {

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


    void Thread::UpdateCPURegisters(CPU_Registers* regs) {
        m_regs = regs;
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
        return m_regs;
    }

    void Thread::Start() {
        g_Scheduler->ScheduleThread(this);
    }

}