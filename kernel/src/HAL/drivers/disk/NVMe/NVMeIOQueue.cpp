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
    }

    NVMeIOQueue::~NVMeIOQueue() {
        if (p_is_created)
            Delete();
    }

    void NVMeIOQueue::Create(void* doorbell_start, uint64_t entry_count, uint8_t ID) {
        if (p_is_created)
            return;
        p_doorbell_start = doorbell_start;
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

    }
}
