#ifndef _HAL_NVME_QUEUE_HPP
#define _HAL_NVME_QUEUE_HPP

#include <stdint.h>

namespace NVMe {

    struct Command {
        uint8_t Opcode;
        uint8_t FusedOperation : 2;
        uint8_t reserved : 4;
        uint8_t PRP_SGL : 2; // PRP or SGL selection. 0 indicates PRPs.
        uint16_t CommandID; // This is put in the completion queue entry.
    } __attribute__((packed));

    struct SubmissionQueueEntry {
        Command command;
        uint32_t NSID; // Namespace identifier. If N/A, set to 0
        uint64_t reserved;
        uint64_t MetadataPTR;
        uint64_t DataPTR0;
        uint64_t DataPTR1;
        uint32_t DWORD10;
        uint32_t DWORD11;
        uint32_t DWORD12;
        uint32_t DWORD13;
        uint32_t DWORD14;
        uint32_t DWORD15;
    } __attribute__((packed));

    struct CompletionQueueEntry {
        uint32_t CommandSpecfic;
        uint32_t Reserved;
        uint16_t SubmissionQueueHeadPTR;
        uint16_t SubmissionQueueID;
        uint16_t CommandID;
        uint8_t Phase : 1; // Toggled when entry written.
        uint16_t Status : 15; // 0 on success
    } __attribute__((packed));

    class NVMeQueue {
    public:
        virtual ~NVMeQueue() {};

        virtual void Create(void* doorbell_start, uint32_t doorbell_stride, uint64_t entry_count, uint8_t ID) = 0;
        virtual void Delete() = 0;
        
        virtual bool SendCommand(const SubmissionQueueEntry* entry) = 0;

        inline virtual SubmissionQueueEntry* GetSubmissionQueue() const { return p_SQEntries; };
        inline virtual CompletionQueueEntry* GetCompletionQueue() const { return p_CQEntries; };
    protected:
        bool p_is_created;
        SubmissionQueueEntry* p_SQEntries;
        CompletionQueueEntry* p_CQEntries;
        uint64_t p_EntryCount;
        uint8_t p_ID;
        void* p_doorbell_start;
        uint32_t p_doorbell_stride;
    };

}

#endif /* _HAL_NVME_QUEUE_HPP */