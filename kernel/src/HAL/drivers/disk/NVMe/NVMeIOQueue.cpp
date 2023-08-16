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

#include "NVMeIOQueue.hpp"

#include <assert.h>
#include <util.h>

#include <Memory/PageManager.hpp>

namespace NVMe {
    
    NVMeIOQueue::NVMeIOQueue() {
        p_is_created = false;
        p_SQEntries = nullptr;
        p_CQEntries = nullptr;
        p_EntryCount = 0;
        p_ID = 0; // Invalid I/O ID
        p_doorbell_start = nullptr;
        p_doorbell_stride = 0;
    }

    NVMeIOQueue::~NVMeIOQueue() {
        if (p_is_created)
            Delete();
    }

    void NVMeIOQueue::Create(void* doorbell_start, uint32_t doorbell_stride, uint64_t entry_count, uint8_t ID) {
        if (p_is_created)
            return;
        p_doorbell_start = doorbell_start;
        p_doorbell_stride = doorbell_stride;
        p_EntryCount = entry_count;
        assert(ID != 0); // ID 0 is reserved for admin queue
        p_ID = ID;
        if ((entry_count * sizeof(CompletionQueueEntry)) <= 0x1000)
            p_CQEntries = (CompletionQueueEntry*)WorldOS::g_KPM->AllocatePage();
        else
            p_CQEntries = (CompletionQueueEntry*)WorldOS::g_KPM->AllocatePages(entry_count * sizeof(CompletionQueueEntry) / 0x1000);
        assert(p_CQEntries != nullptr);
        fast_memset(p_CQEntries, 0, sizeof(CompletionQueueEntry) * entry_count / 8);
        if ((entry_count * sizeof(SubmissionQueueEntry)) <= 0x1000)
            p_SQEntries = (SubmissionQueueEntry*)WorldOS::g_KPM->AllocatePage();
        else
            p_SQEntries = (SubmissionQueueEntry*)WorldOS::g_KPM->AllocatePages(entry_count * sizeof(SubmissionQueueEntry) / 0x1000);
        assert(p_SQEntries != nullptr);
        fast_memset(p_SQEntries, 0, sizeof(SubmissionQueueEntry) * entry_count / 8);
        p_is_created = true;
    }

    void NVMeIOQueue::Delete() {
        if (!p_is_created)
            return;
        if ((p_EntryCount * sizeof(CompletionQueueEntry)) <= 0x1000)
            WorldOS::g_KPM->FreePage(p_CQEntries);
        else
            WorldOS::g_KPM->FreePages(p_CQEntries);
        if ((p_EntryCount * sizeof(SubmissionQueueEntry)) <= 0x1000)
            WorldOS::g_KPM->FreePage(p_SQEntries);
        else
            WorldOS::g_KPM->FreePages(p_SQEntries);
        p_is_created = false;
        p_SQEntries = nullptr;
        p_CQEntries = nullptr;
        p_EntryCount = 0;
        p_ID = 0; // Invalid IO
        p_doorbell_start = nullptr;
    }
    
    bool NVMeIOQueue::SendCommand(const SubmissionQueueEntry* entry) {
        if (!p_is_created)
            return false;
        uint32_t* admin_tail = reinterpret_cast<uint32_t*>(p_doorbell_start + 2 * p_ID * p_doorbell_stride);
        const uint32_t old_admin_tail = *admin_tail;
        fast_memcpy((void*)((uint64_t)p_SQEntries + old_admin_tail * sizeof(SubmissionQueueEntry)), entry, sizeof(SubmissionQueueEntry));
        if (old_admin_tail >= p_EntryCount)
            *admin_tail = 0;
        else
            (*admin_tail)++;
        CompletionQueueEntry* CQ = nullptr;
        do {
            CQ = reinterpret_cast<CompletionQueueEntry*>((uint64_t)p_CQEntries + old_admin_tail * sizeof(CompletionQueueEntry));
        } while (CQ->Phase);
        bool Successful = CQ->Status == 0;
        fast_memset(CQ, 0, sizeof(CompletionQueueEntry) / 8);
        return Successful;
    }
}
