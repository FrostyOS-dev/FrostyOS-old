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