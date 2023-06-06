#ifndef _HAL_NVME_HPP
#define _HAL_NVME_HPP

#include "Disk.hpp"
#include "../PCIDevice.hpp"

class NVMeDisk : public Disk, public PCIDevice {
public:
    NVMeDisk();
    ~NVMeDisk() override;

    void InitPCIDevice(PCI::Header0* device) override;

    void Read(uint8_t* buffer, uint64_t lba, uint64_t count = 1) override;
    void Write(const uint8_t* buffer, uint64_t lba, uint64_t count = 1) override;

    size_t GetSectorSize() override;

    const char* getVendorName() override;
    const char* getDeviceName() override;

    const char* getDeviceClass() override;
    const char* getDeviceSubClass() override;
    const char* getDeviceProgramInterface() override;

protected:
    void AdminCommand(void* SBQEntry);

private:
    void* m_BAR0;
    uint8_t m_IRQ;
    void* m_ASQ_address;
    void* m_ACQ_address;
    void* m_IOSQ_address;
    void* m_IOCQ_address;
    void* m_ControllerInfo;
    void* m_NSIDList;
    void* m_NSID1Info;
};

#endif /* _HAL_NVME_HPP */