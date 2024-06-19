/*
Copyright (©) 2023-2024  Frosty515

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

#include "exit.hpp"

#include <Scheduling/Scheduler.hpp>

#ifdef __x86_64__
#include <arch/x86_64/Memory/PagingUtil.hpp>
#include <arch/x86_64/Memory/PageMapIndexer.hpp>
#include <arch/x86_64/Scheduling/taskutil.hpp>

#include <arch/x86_64/Stack.hpp>
#endif

#include <util.h>

void do_exit(Scheduling::Thread* thread, int status, bool was_scheduler_running) {
    using namespace Scheduling;
    Process* parent = thread->GetParent();
#ifdef __x86_64__
    x86_64_LoadCR3((uint64_t)(g_KPM->GetPageTable().GetRootTablePhysical()) & 0x000FFFFFFFFFF000); // reset the CR3 value so we are not using an address space that it being destroyed
#endif
    if (parent == nullptr) {
        PANIC("Parent-less thread exiting.");
    }
    Scheduling::Scheduler::RemoveThread(thread);
    if (thread->GetFlags() & CREATE_STACK)
        parent->GetPageManager()->FreePages((void*)(thread->GetStack() - KiB(64)));
    ThreadCleanup_t cleanup = thread->GetCleanupFunction();
    bool is_main_thread = parent->GetMainThread() == thread;
    parent->RemoveThread(thread);
    if (is_main_thread) {
        for (uint64_t i = 0; i < parent->GetThreadCount(); i++) {
            Thread* i_thread = parent->GetThread(0);
            if (i_thread == nullptr)
                break; // should never happen
            Scheduler::RemoveThread(i_thread);
            if (i_thread->GetFlags() & CREATE_STACK)
                parent->GetPageManager()->FreePages((void*)(i_thread->GetStack() - KiB(64)));
            ThreadCleanup_t cleanup = i_thread->GetCleanupFunction();
            if (cleanup.function != nullptr)
                cleanup.function(cleanup.data);
            parent->RemoveThread(i_thread);
            delete i_thread;
        }
        delete parent;
    }
    delete thread;
    if (cleanup.function != nullptr)
        cleanup.function(cleanup.data);
    if (was_scheduler_running)
        Scheduler::Resume();
}

void sys_exit(Scheduling::Thread* thread, int status) {
    using namespace Scheduling;
    Thread* current_thread = Scheduler::GetCurrent();
    bool was_running = Scheduler::isRunning();
    if (was_running)
        Scheduler::Stop();
    if (thread != current_thread) // unhandled signal received from different thread most likely.
        do_exit(thread, status, was_running);
#ifdef __x86_64__
    x86_64_PrepareThreadExit(thread, status, was_running, do_exit);
#endif
}
