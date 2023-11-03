/*
Copyright (©) 2023  Frosty515

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
#endif

void sys$exit(Scheduling::Thread* thread, int status) {
    //(void)status; // has no meaning currently
    using namespace Scheduling;
    Process* parent = thread->GetParent();
#ifdef __x86_64__
    x86_64_LoadCR3((uint64_t)(g_KPM->GetPageTable().GetRootTablePhysical()) & 0x000FFFFFFFFFF000); // reset the CR3 value so we are not using an address space that it being destroyed
#endif
    if (parent == nullptr) {
        ThreadCleanup_t cleanup = thread->GetCleanupFunction();
        delete thread;
        if (cleanup.function != nullptr)
            cleanup.function(cleanup.data);
        return;
    }
    bool was_running = Scheduler::isRunning();
    if (was_running)
        Scheduler::Stop();
    if (thread->GetFlags() & CREATE_STACK)
        parent->GetPageManager()->FreePages((void*)(thread->GetStack() - KiB(64)));
    ThreadCleanup_t cleanup = thread->GetCleanupFunction();
    Scheduling::Scheduler::RemoveThread(thread);
    bool is_main_thread = parent->GetMainThread() == thread;
    parent->RemoveThread(thread);
    if (is_main_thread) {
        for (uint64_t i = 0; i < parent->GetThreadCount(); i++) {
            Thread* i_thread = parent->GetThread(0);
            if (i_thread == nullptr)
                break; // should never happen
            if (i_thread->GetFlags() & CREATE_STACK)
                parent->GetPageManager()->FreePages((void*)(i_thread->GetStack() - KiB(64)));
            cleanup = i_thread->GetCleanupFunction();
            if (cleanup.function != nullptr)
                cleanup.function(cleanup.data);
            Scheduler::RemoveThread(i_thread);
            parent->RemoveThread(i_thread);
            delete i_thread;
        }
        delete parent;
    }
    delete thread;
    if (cleanup.function != nullptr)
        cleanup.function(cleanup.data);
    if (was_running)
        Scheduler::Resume();
}
