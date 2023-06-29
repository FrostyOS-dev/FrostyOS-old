#ifndef _HAL_NVME_IO_QUEUE_HPP
#define _HAL_NVME_IO_QUEUE_HPP

#include "NVMeQueue.hpp"

namespace NVMe {

    enum class IOCommands {
        READ = 0x02,
        WRITE = 0x01
    };

    class NVMeIOQueue : public NVMeQueue {
    public:
        NVMeIOQueue();
        ~NVMeIOQueue() override;

        void Create(void* doorbell_start, uint32_t doorbell_stride, uint64_t entry_count, uint8_t ID) override;
        void Delete() override;

        bool SendCommand(const SubmissionQueueEntry* entry) override;
    };
}

#endif /* _HAL_NVME_IO_QUEUE_HPP */