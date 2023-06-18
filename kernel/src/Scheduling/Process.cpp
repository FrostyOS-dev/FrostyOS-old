#include "Process.hpp"

#include "Scheduler.hpp"

namespace Scheduling {
    Process::Process() : m_Entry(nullptr), m_entry_data(nullptr), m_flags(USER_DEFAULT), m_Priority(Priority::NORMAL), m_pm(nullptr), m_main_thread_initialised(false), m_main_thread(nullptr) {

    }

    Process::Process(ProcessEntry_t entry, void* entry_data, Priority priority, uint8_t flags, WorldOS::PageManager* pm) : m_Entry(entry), m_entry_data(entry_data), m_flags(flags), m_Priority(priority), m_pm(pm), m_main_thread_initialised(false), m_main_thread(nullptr) {

    }

    Process::~Process() {
        /*m_threads.remove(m_main_thread);
        delete m_main_thread;
        m_main_thread_initialised = false;*/
    }

    void Process::SetEntry(ProcessEntry_t entry, void* entry_data) {
        m_Entry = entry;
        m_entry_data = entry_data;
    }

    void Process::SetPriority(Priority priority) {
        m_Priority = priority;
    }

    void Process::SetFlags(uint8_t flags) {
        m_flags = flags;
    }

    void Process::SetPageManager(WorldOS::PageManager* pm) {
        m_pm = pm;
    }

    ProcessEntry_t Process::GetEntry() const {
        return m_Entry;
    }

    void* Process::GetEntryData() const {
        return m_entry_data;
    }

    Priority Process::GetPriority() const {
        return m_Priority;
    }

    uint8_t Process::GetFlags() const {
        return m_flags;
    }

    WorldOS::PageManager* Process::GetPageManager() const {
        return m_pm;
    }

    void Process::Start() {
        if (m_main_thread_initialised)
            return;
        g_Scheduler->AddProcess(this);
        m_main_thread = new Thread(this, m_Entry, m_entry_data, m_flags);
        m_threads.insert(m_main_thread);
        m_main_thread_initialised = true;
        m_main_thread->Start();
    }

    void Process::ScheduleThread(Thread* thread) {
        if (thread == nullptr)
            return;
        m_threads.insert(thread);
        thread->SetParent(this);
        thread->Start();
    }
}
