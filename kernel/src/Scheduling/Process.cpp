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

#include "Process.hpp"

#include "Scheduler.hpp"

#include <Memory/VirtualPageManager.hpp>

namespace Scheduling {
    Process::Process() : m_Entry(nullptr), m_entry_data(nullptr), m_flags(USER_DEFAULT), m_Priority(Priority::NORMAL), m_pm(nullptr), m_main_thread_initialised(false), m_main_thread(nullptr) {

    }

    Process::Process(ProcessEntry_t entry, void* entry_data, Priority priority, uint8_t flags, WorldOS::PageManager* pm) : m_Entry(entry), m_entry_data(entry_data), m_flags(flags), m_Priority(priority), m_pm(pm), m_main_thread_initialised(false), m_main_thread(nullptr) {

    }

    Process::~Process() {
        delete m_main_thread;
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

    void Process::SetRegion(const WorldOS::VirtualRegion& region) {
        m_region = region;
    }

    void Process::SetVirtualPageManager(WorldOS::VirtualPageManager* VPM) {
        m_VPM = VPM;
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

    const WorldOS::VirtualRegion& Process::GetRegion() const {
        return m_region;
    }

    WorldOS::VirtualPageManager* Process::GetVirtualPageManager() const {
        return m_VPM;
    }

    void Process::Start() {
        if (m_main_thread_initialised)
            return;
        Scheduler::AddProcess(this);
        if (m_flags & ALLOCATE_VIRTUAL_SPACE && m_pm == nullptr && m_VPM == nullptr) {
            m_region = WorldOS::VirtualRegion(WorldOS::g_VPM->AllocatePages(MiB(16) >> 12), MiB(16));
            m_VPM = new WorldOS::VirtualPageManager;
            m_VPM->InitVPageMgr(m_region);
            m_pm = new WorldOS::PageManager(m_region, m_VPM, m_Priority != Priority::KERNEL);
        }
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
