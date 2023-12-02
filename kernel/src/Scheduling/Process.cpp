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
    Process::Process() : m_Entry(nullptr), m_entry_data(nullptr), m_flags(USER_DEFAULT), m_Priority(Priority::NORMAL), m_pm(nullptr), m_main_thread_initialised(false), m_main_thread(nullptr), m_main_thread_creation_requested(false), m_region_allocated(false), m_UID(0), m_GID(0), m_EUID(0), m_EGID(0) {

    }

    Process::Process(ProcessEntry_t entry, void* entry_data, uint32_t UID, uint32_t GID, Priority priority, uint8_t flags, PageManager* pm) : m_Entry(entry), m_entry_data(entry_data), m_flags(flags), m_Priority(priority), m_pm(pm), m_main_thread_initialised(false), m_main_thread(nullptr), m_main_thread_creation_requested(false), m_region_allocated(false), m_UID(UID), m_GID(GID), m_EUID(UID), m_EGID(GID) {

    }

    Process::~Process() {
        delete m_main_thread;
        if (m_region_allocated) {
            delete m_pm;
            delete m_VPM;
        }
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

    void Process::SetPageManager(PageManager* pm) {
        m_pm = pm;
    }

    void Process::SetRegion(const VirtualRegion& region) {
        m_region = region;
    }

    void Process::SetVirtualPageManager(VirtualPageManager* VPM) {
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

    PageManager* Process::GetPageManager() const {
        return m_pm;
    }

    const VirtualRegion& Process::GetRegion() const {
        return m_region;
    }

    VirtualPageManager* Process::GetVirtualPageManager() const {
        return m_VPM;
    }

    Thread* Process::GetMainThread() const {
        return m_main_thread_initialised ? m_main_thread : nullptr;
    }

    uint64_t Process::GetThreadCount() const {
        return m_threads.getCount();
    }

    Thread* Process::GetThread(uint64_t index) const {
        return m_threads.get(index);
    }

    void Process::CreateMainThread() {
        m_main_thread = new Thread(this, m_Entry, m_entry_data, m_flags);
        m_threads.insert(m_main_thread);
        m_main_thread_initialised = true;
        m_main_thread_creation_requested = true;
    }

    void Process::Start() {
        if (m_main_thread_initialised && !m_main_thread_creation_requested)
            return;
        Scheduler::AddProcess(this);
        if (m_flags & ALLOCATE_VIRTUAL_SPACE && m_pm == nullptr && m_VPM == nullptr) {
            m_region = VirtualRegion(g_VPM->AllocatePages(MiB(16) >> 12), MiB(16));
            m_VPM = new VirtualPageManager;
            m_VPM->InitVPageMgr(m_region);
            m_pm = new PageManager(m_region, m_VPM, m_Priority != Priority::KERNEL);
            m_region_allocated = true;
        }
        else
            m_region = m_pm->GetRegion(); // ensure the region is up to date
        if (!m_main_thread_initialised) {
            m_main_thread = new Thread(this, m_Entry, m_entry_data, m_flags);
            m_threads.insert(m_main_thread);
            m_main_thread_initialised = true;
        }
        m_main_thread->Start();
    }

    void Process::ScheduleThread(Thread* thread) {
        if (thread == nullptr)
            return;
        m_threads.insert(thread);
        thread->SetParent(this);
        thread->Start();
    }

    void Process::RemoveThread(Thread* thread) {
        if (thread == nullptr)
            return;
        uint64_t i = m_threads.getIndex(thread); // verify the thread is actually valid
        if (i == UINT64_MAX)
            return;
        bool is_main_thread = m_main_thread == thread;
        m_threads.remove(thread);
        thread->SetParent(nullptr);
        if (is_main_thread)
            m_main_thread = nullptr;
    }

    void Process::RemoveThread(uint64_t index) {
        Thread* thread = m_threads.get(index);
        if (thread == nullptr)
            return;
        m_threads.remove(index);
        thread->SetParent(nullptr);
    }

    bool Process::ValidateRead(const void* buf, size_t size) const {
        return m_region.IsInside(buf, size);
    }

    bool Process::ValidateStringRead(const char* str) const {
        if (!m_region.IsInside(str, sizeof(char)))
            return false;
        if (str[0] == '\0')
            return true;
        return ValidateStringRead((const char*)((uint64_t)str + sizeof(char)));
    }
    
    bool Process::ValidateWrite(void* buf, size_t size) const {
        if (!m_region.IsInside(buf, size))
            return false;
        if (m_pm == nullptr)
            return false; // no way to check
        return m_pm->isWritable(buf, size);
    }

    void Process::SyncRegion() {
        m_region = m_pm->GetRegion();
    }

    void Process::SetUID(uint32_t uid) {
        m_UID = uid;
        m_EUID = uid;
    }

    void Process::SetGID(uint32_t gid) {
        m_GID = gid;
        m_EGID = gid;
    }

    void Process::SetEUID(uint32_t euid) {
        m_EUID = euid;
    }

    void Process::SetEGID(uint32_t egid) {
        m_EGID = egid;
    }

    uint32_t Process::GetUID() const {
        return m_UID;
    }

    uint32_t Process::GetGID() const {
        return m_GID;
    }

    uint32_t Process::GetEUID() const {
        return m_EUID;
    }

    uint32_t Process::GetEGID() const {
        return m_EGID;
    }
}
