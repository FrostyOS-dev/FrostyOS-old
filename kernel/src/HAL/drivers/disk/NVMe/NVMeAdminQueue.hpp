#ifndef _HAL_NVME_ADMIN_QUEUE_HPP
#define _HAL_NVME_ADMIN_QUEUE_HPP

#include "NVMeQueue.hpp"

namespace NVMe {

    enum class AdminCommands {
        CREATE_IO_SUBMISSION_QUEUE = 0x01,
        CREATE_IO_COMPLETION_QUEUE = 0x05,
        IDENTIFY = 0x06
    };

    class NVMeAdminQueue : public NVMeQueue {
    public:
        NVMeAdminQueue();
        ~NVMeAdminQueue() override;

        void Create(void* doorbell_start, uint32_t doorbell_stride, uint64_t entry_count, uint8_t ID) override;
        void Delete() override;
        
        bool SendCommand(const SubmissionQueueEntry* entry) override;
    };
}

#endif /* _HAL_NVME_ADMIN_QUEUE_HPP */