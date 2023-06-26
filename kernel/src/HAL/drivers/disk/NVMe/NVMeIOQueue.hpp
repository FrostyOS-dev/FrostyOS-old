#ifndef _HAL_NVME_IO_QUEUE_HPP
#define _HAL_NVME_IO_QUEUE_HPP

#include "NVMeQueue.hpp"

namespace NVMe {
    class NVMeIOQueue : public NVMeQueue {
    public:
        NVMeIOQueue();
        ~NVMeIOQueue() override;

        void Create(void* doorbell_start, uint64_t entry_count, uint8_t ID) override;
        void Delete() override;

        bool SendCommand(const SubmissionQueueEntry* entry) override;
    };
}

#endif /* _HAL_NVME_IO_QUEUE_HPP */