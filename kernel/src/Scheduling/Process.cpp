/*
Copyright (©) 2022-2024  Frosty515

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

#include <errno.h>

#include <SystemCalls/exit.hpp>

#include <Memory/VirtualPageManager.hpp>

#ifdef __x86_64__
#include <arch/x86_64/Scheduling/taskutil.hpp>
#endif

namespace Scheduling {
    bool PriorityLessThan(const Priority& lhs, const Priority& rhs) {
        if (lhs == rhs)
            return false;
        if (lhs == Priority::KERNEL) // impossible for rhs to be greater
            return false;
        if (rhs == Priority::KERNEL) // impossible for lhs to be greater or equal
            return true;
        if (rhs == Priority::HIGH) // lhs is either NORMAL or LOW if this is true
            return true;
        if (rhs == Priority::NORMAL && lhs == Priority::LOW)
            return true;
        return false;
    }

    bool PriorityGreaterThan(const Priority& lhs, const Priority& rhs) {
        if (lhs == rhs)
            return false;
        if (lhs == Priority::KERNEL) // impossible for rhs to be less or equal
            return true;
        if (rhs == Priority::KERNEL) // impossible for lhs to be greater or equal
            return false;
        if (lhs == Priority::HIGH) // rhs is either NORMAL or LOW if this is true
            return true;
        if (lhs == Priority::NORMAL && rhs == Priority::LOW)
            return true;
        return false;
    }

    bool PriorityLessOrEqual(const Priority& lhs, const Priority& rhs) {
        if (lhs == rhs)
            return true;
        if (lhs == Priority::KERNEL) // impossible for rhs to be greater
            return true;
        if (rhs == Priority::KERNEL) // impossible for lhs to be greater or equal
            return false;
        if (rhs == Priority::HIGH) // lhs is either NORMAL or LOW if this is true
            return true;
        if (rhs == Priority::NORMAL && lhs == Priority::LOW)
            return true;
        return false;
    }

    bool PriorityGreaterOrEqual(const Priority& lhs, const Priority& rhs) {
        if (lhs == rhs)
            return true;
        if (lhs == Priority::KERNEL) // impossible for rhs to be less or equal
            return false;
        if (rhs == Priority::KERNEL) // impossible for lhs to be greater or equal
            return true;
        if (lhs == Priority::HIGH) // rhs is either NORMAL or LOW if this is true
            return true;
        if (lhs == Priority::NORMAL && rhs == Priority::LOW)
            return true;
        return false;
    }



    Process::Process() : m_Entry(nullptr), m_entry_data(nullptr), m_flags(USER_DEFAULT), m_Priority(Priority::NORMAL), m_pm(nullptr), m_main_thread_initialised(false), m_main_thread(nullptr), m_region(nullptr, nullptr), m_VPM(nullptr), m_main_thread_creation_requested(false), m_region_allocated(false), m_PID(-1), m_NextTID(0), m_UID(0), m_GID(0), m_EUID(0), m_EGID(0), m_defaultWorkingDirectory(nullptr), m_sigMetadata{{false, nullptr, 0, nullptr, nullptr, 0}} {

    }

    Process::Process(ProcessEntry_t entry, void* entry_data, uint32_t UID, uint32_t GID, Priority priority, uint8_t flags, PageManager* pm) : m_Entry(entry), m_entry_data(entry_data), m_flags(flags), m_Priority(priority), m_pm(pm), m_main_thread_initialised(false), m_main_thread(nullptr), m_main_thread_creation_requested(false), m_region_allocated(false), m_UID(UID), m_GID(GID), m_EUID(UID), m_EGID(GID), m_defaultWorkingDirectory(nullptr), m_sigMetadata{{false, nullptr, 0, nullptr, nullptr, 0}} {

    }

    Process::~Process() {
        delete m_main_thread;
        if (m_region_allocated) {
            delete m_pm;
            delete m_VPM;
        }
        if (m_defaultWorkingDirectory != nullptr)
            delete m_defaultWorkingDirectory;
        for (uint64_t i = 0; i < (SIG_MAX - SIG_MIN + 1); i++) {
            if (m_sigMetadata[i].in_signal_handler) {
                delete m_sigMetadata[i].old_regs;
                delete[] (uint8_t*)(m_sigMetadata[i].old_ins);
            }
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
        m_main_thread = new Thread(this, m_Entry, m_entry_data, m_flags, m_NextTID);
        m_NextTID++;
        if (m_main_thread->GetWorkingDirectory() == nullptr && m_defaultWorkingDirectory != nullptr)
            m_main_thread->SetWorkingDirectory(new VFS_WorkingDirectory(*m_defaultWorkingDirectory));
        m_threads.insert(m_main_thread);
        m_main_thread_initialised = true;
        m_main_thread_creation_requested = true;
    }

    void Process::Start() {
        if (m_main_thread_initialised && !m_main_thread_creation_requested)
            return;
        for (uint64_t i = 0; i < (SIG_MAX - SIG_MIN + 1); i++)
            m_sigActions[i].flags = SIG_DFL;
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
            m_main_thread = new Thread(this, m_Entry, m_entry_data, m_flags, m_NextTID);
            m_NextTID++;
            if (m_main_thread->GetWorkingDirectory() == nullptr && m_defaultWorkingDirectory != nullptr)
                m_main_thread->SetWorkingDirectory(new VFS_WorkingDirectory(*m_defaultWorkingDirectory));
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
        thread->SetTID(m_NextTID);
        m_NextTID++;
        if (thread->GetWorkingDirectory() == nullptr && m_defaultWorkingDirectory != nullptr)
            thread->SetWorkingDirectory(new VFS_WorkingDirectory(*m_defaultWorkingDirectory));
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

    bool Process::IsMainThread(Thread* thread) const {
        if (thread == nullptr || !m_main_thread_initialised || m_main_thread == nullptr)
            return false;
        return thread == m_main_thread;
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

    void Process::SetPID(pid_t pid) {
        m_PID = pid;
    }

    pid_t Process::GetPID() const {
        return m_PID;
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

    void Process::SetDefaultWorkingDirectory(VFS_WorkingDirectory* wd) {
        m_defaultWorkingDirectory = wd;
    }

    VFS_WorkingDirectory* Process::GetDefaultWorkingDirectory() const {
        return m_defaultWorkingDirectory;
    }

    int Process::sys_onsignal(int signum, const struct signal_action* new_action, struct signal_action* old_action) {
        if (!IN_BOUNDS(signum, SIG_MIN, SIG_MAX))
            return -EINVAL;
        if (old_action != nullptr) {
            if (!ValidateWrite(old_action, sizeof(struct signal_action)))
                return -EFAULT;
            memcpy(old_action, &(m_sigActions[signum - SIG_MIN]), sizeof(struct signal_action));
        }
        if (new_action != nullptr) {
            if (!ValidateRead(new_action, sizeof(struct signal_action)))
                return -EFAULT;
            memcpy(&(m_sigActions[signum - SIG_MIN]), new_action, sizeof(struct signal_action));
        }
        return ESUCCESS;
    }

    int Process::sys_sendsig(pid_t pid, int signum) {
        return Scheduler::SendSignal(this, pid, signum);
    }

    void Process::sys_sigreturn(int signum) {
        // We must first restore the registers
        CPU_Registers* regs = m_main_thread->GetCPURegisters();
        memcpy(regs, m_sigMetadata[signum - SIG_MIN].old_regs, sizeof(CPU_Registers));
        delete m_sigMetadata[signum - SIG_MIN].old_regs;
#ifdef __x86_64__
        // We must now restore the instructions
        // Validate we can read the pages
        if (!ValidateRead((void*)regs->RIP, m_sigMetadata[signum - SIG_MIN].ret_ins_size)) {
            // If we are in a SIGSEGV handler, we set the action for SIGSEGV to default, and then send it
            if (signum == SIGSEGV)
                m_sigActions[SIGSEGV - SIG_MIN].flags = SIG_DFL;
            ReceiveSignal(SIGSEGV);
        }
        uint64_t start = ALIGN_DOWN(regs->RIP, PAGE_SIZE);
        uint64_t end = ALIGN_UP((regs->RIP + m_sigMetadata[signum - SIG_MIN].ret_ins_size), PAGE_SIZE);
        uint64_t pages = (end - start) / PAGE_SIZE;
        PagePermissions* perms = new PagePermissions[pages];
        for (uint64_t i = 0; i < pages; i++) {
            perms[i] = m_pm->GetPermissions((void*)(start + (i * PAGE_SIZE)));
            m_pm->Remap((void*)(start + (i * PAGE_SIZE)), PagePermissions::READ_WRITE);
        }
        memcpy((void*)regs->RIP, m_sigMetadata[signum - SIG_MIN].old_ins, m_sigMetadata[signum - SIG_MIN].ret_ins_size);
        delete[] (uint8_t*)(m_sigMetadata[signum - SIG_MIN].old_ins);
        // restore the permissions
        for (uint64_t i = 0; i < pages; i++)
            m_pm->Remap((void*)(start + (i * PAGE_SIZE)), perms[i]);
        delete[] perms;
        *(uint64_t*)(regs->RSP - 8) = (uint64_t)m_sigMetadata[signum - SIG_MIN].extra_data;
#endif
        m_sigMetadata[signum - SIG_MIN].in_signal_handler = false;
        // we are in a syscall, so we must force the scheduler to pick a different thread to run next
        Scheduler::PickNext();
        Scheduler::Next();
        // we should never reach here
        PANIC("sys_sigreturn returned");
    }

    void Process::ReceiveSignal(int signum) {
        struct signal_action action = m_sigActions[signum - SIG_MIN];
        if (action.flags == SIG_IGN)
            return;
        if (action.flags == SIG_DFL) {
            switch (signum) {
            case SIGABRT:
            case SIGFPE:
            case SIGILL:
            case SIGINT:
            case SIGSEGV:
            case SIGTERM:
            case SIGKILL:
                sys_exit(m_main_thread, signum + SIG_RET_STATUS_OFFSET); // we don't return from this.
                break;
            case SIGCHLD:
                break;
            default:
                break;
            }
        }
        else {
            /*
            This is where things get complicated.
            We need to save the current state of the thread in a way where if it exits, we can still cleanup the thread,
            and then move the thread to the signal handler, get it to return to here, while exiting userland, and then restore the thread state.
            To do this, we copy some special return instructions to the location at which the thread was interrupt, after saving the previous state.
            */
            if (m_sigMetadata[signum - SIG_MIN].in_signal_handler)
                return; // we're already in a signal handler, so we can't handle this signal.
            CPU_Registers* signal_regs = new CPU_Registers;
            memcpy(signal_regs, m_main_thread->GetCPURegisters(), sizeof(CPU_Registers));
            m_sigMetadata[signum - SIG_MIN].old_regs = signal_regs;
            m_sigMetadata[signum - SIG_MIN].in_signal_handler = true;
#ifdef __x86_64__
            m_sigMetadata[signum - SIG_MIN].ret_ins = x86_64_GetSignalReturnInstructions(&(m_sigMetadata[signum - SIG_MIN].ret_ins_size), signum);
            CPU_Registers* regs = m_main_thread->GetCPURegisters();
            // We must validate that we can decrement the stack
            if (!ValidateWrite((void*)(regs->RSP - 8), 8)) {
                // If we are in a SIGSEGV handler, we set the action for SIGSEGV to default, and then send it
                if (signum == SIGSEGV)
                    m_sigActions[SIGSEGV - SIG_MIN].flags = SIG_DFL;
                delete signal_regs;
                m_sigMetadata[signum - SIG_MIN].in_signal_handler = false;
                ReceiveSignal(SIGSEGV);
            }
            // push the current RIP onto the stack
            regs->RSP -= 8;
            m_sigMetadata[signum - SIG_MIN].extra_data = *(uint64_t*)(regs->RSP); // save it
            *(uint64_t*)(regs->RSP) = regs->RIP;
            // we must now verify that the RIP is readable. If it is not writable, we just make it writable temporarily, and then copy.
            if (!ValidateRead((void*)regs->RIP, m_sigMetadata[signum - SIG_MIN].ret_ins_size)) {
                // If we are in a SIGSEGV handler, we set the action for SIGSEGV to default, and then send it
                if (signum == SIGSEGV)
                    m_sigActions[SIGSEGV - SIG_MIN].flags = SIG_DFL;
                delete signal_regs;
                *(uint64_t*)(regs->RSP) = m_sigMetadata[signum - SIG_MIN].extra_data;
                regs->RSP += 8;
                m_sigMetadata[signum - SIG_MIN].in_signal_handler = false;
                ReceiveSignal(SIGSEGV);
            }
            m_sigMetadata[signum - SIG_MIN].old_ins = new uint8_t[m_sigMetadata[signum - SIG_MIN].ret_ins_size];
            memcpy(m_sigMetadata[signum - SIG_MIN].old_ins, (void*)regs->RIP, m_sigMetadata[signum - SIG_MIN].ret_ins_size);
            // we must now enumerate the pages that we need to make writable
            uint64_t start = ALIGN_DOWN(regs->RIP, PAGE_SIZE);
            uint64_t end = ALIGN_UP((regs->RIP + m_sigMetadata[signum - SIG_MIN].ret_ins_size), PAGE_SIZE);
            uint64_t pages = (end - start) / PAGE_SIZE;
            PagePermissions* perms = new PagePermissions[pages];
            for (uint64_t i = 0; i < pages; i++) {
                perms[i] = m_pm->GetPermissions((void*)(start + (i * PAGE_SIZE)));
                m_pm->Remap((void*)(start + (i * PAGE_SIZE)), PagePermissions::READ_WRITE);
            }
            memcpy((void*)regs->RIP, m_sigMetadata[signum - SIG_MIN].ret_ins, m_sigMetadata[signum - SIG_MIN].ret_ins_size);
            // restore the permissions
            for (uint64_t i = 0; i < pages; i++)
                m_pm->Remap((void*)(start + (i * PAGE_SIZE)), perms[i]);
            delete[] perms;
            delete[] (uint8_t*)(m_sigMetadata[signum - SIG_MIN].ret_ins);
            regs->RIP = (uint64_t)action.handler;
            regs->RDI = (uint64_t)signum;
#endif
            /*
            if the main thread is the current thread, a.k.a. we are in a syscall, 
            we have to force the scheduler to pick a different thread to run next
            */
            if (m_main_thread == Scheduler::GetCurrent()) {
                Scheduler::PickNext();
                Scheduler::Next();
                // we should never get to here
                PANIC("Scheduler::Next returned after signal received on current thread.");
            }

        }
    }

    bool Process::IsInSignalHandler(int signum) const {
        return m_sigMetadata[signum - SIG_MIN].in_signal_handler;
    }

}
