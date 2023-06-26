#ifndef _HAL_NVME_HPP
#define _HAL_NVME_HPP

#include "../Disk.hpp"
#include "../../PCIDevice.hpp"

#include "NVMeIOQueue.hpp"

namespace NVMe {
    class NVMeController;

    class NVMeDisk : public Disk {
    public:
        NVMeDisk(uint32_t ID, NVMeController* controller, NVMeIOQueue* IOQueue);
        ~NVMeDisk() override;

        void Read(uint8_t* buffer, uint64_t lba, uint64_t count = 1) override;
        void Write(const uint8_t* buffer, uint64_t lba, uint64_t count = 1) override;

        size_t GetSectorSize() override;

    private:
        uint32_t m_ID;
        NVMeController* m_controller;
        NVMeIOQueue* m_IOQueue;
    };

}

#endif /* _HAL_NVME_HPP */