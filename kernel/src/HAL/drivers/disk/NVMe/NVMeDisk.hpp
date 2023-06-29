#ifndef _HAL_NVME_HPP
#define _HAL_NVME_HPP

#include "../Disk.hpp"
#include "../../PCIDevice.hpp"

#include "NVMeIOQueue.hpp"

namespace NVMe {

    struct LBAFormat {
        uint16_t MS; // Metadata Size
        uint8_t LBADS; // LBA Data Size. Given as a power of 2. Any non-zero value below 9 is not supported. Value of 0 means LBA Format is unavailable.
        uint8_t RP : 2; // Relative Performance. 0b00 for best, 0b01 for better, 0b10 for good, 0b11 for degraded performance
        uint8_t Reserved : 6;
    } __attribute__((packed));

    struct IdentifyNamespace {
        uint64_t NSZE; // Namespace Size
        uint64_t NCAP; // Namespace Capacity
        uint64_t NUSE; // Namespace Utilization
        uint8_t NSFEAT; // Namespace Features
        uint8_t NLBAF; // Number of LBA Formats
        uint8_t FLBAS; // Formatted LBA Size
        uint8_t MC; // Metadata Capabilities
        uint8_t DPC; // End-to-end Data Protection Capabilities
        uint8_t DPS; // End-to-end Data Protection Type Settings
        uint8_t NMIC; // Namespace Multi-path I/O and Namespace Sharing Capabilities
        uint8_t RESCAP; // Reservation Capabilities
        uint8_t FPI; // Format Progress Indicator
        uint8_t DLFEAT; // Deallocate Logical Block Features
        uint16_t NAWUN; // Namespace Atomic Write Unit Normal
        uint16_t NAWUPF; // Namespace Atomic Write Unit Power Fail
        uint16_t NACWU; // Namespace Atomic Compare & Write Unit
        uint16_t NABSN; // Namespace Atomic Boundary Size Normal
        uint16_t NABO; // Namespace Atomic Boundary Offset
        uint16_t NABSPF; // Namespace Atomic Boundary Size Power Fail
        uint16_t NOIOB; // Namespace Optimal I/O Boundary
        uint64_t NVMCAP[2]; // NVM Capacity
        uint16_t NPWG; // Namespace Preferred Write Granularity
        uint16_t NPWA; // Namespace Preferred Write Alignment
        uint16_t NPDG; // Namespace Preferred Deallocate Granularity
        uint16_t NPDA; // Namespace Preferred Deallocate Alignment
        uint16_t NOWS; // Namespace Optimal Write Size
        uint16_t MSSRL; // Maximum Single Source Range Length
        uint32_t MCL; // Maximum Copy Length
        uint8_t MSRC; // Maximum Source Range Count
        uint8_t Reserved0[11];
        uint32_t ANAGRPID; // ANA Group Identifier
        uint8_t Reserved1[3];
        uint8_t NSATTR; // Namespace Attributes
        uint16_t NVMSETID; // NVM Set Identifier
        uint16_t ENGID; // Endurance Group Identifier
        uint64_t NGUID[2]; // Namespace Globally Unique Identifier
        uint64_t EUI64; // IEEE Extended Unique Identifier
        LBAFormat LBAF[64]; // LBA Format 0-63 Support
        uint8_t VendorSpecific[3712];
    } __attribute__((packed));

    class NVMeController;

    class NVMeDisk : public Disk {
    public:
        NVMeDisk(uint32_t ID, NVMeController* controller, NVMeIOQueue* IOQueue, size_t MaxTransferSize);
        ~NVMeDisk() override;

        bool Read(uint8_t* buffer, uint64_t lba, uint64_t count = 1) override;
        bool Write(const uint8_t* buffer, uint64_t lba, uint64_t count = 1) override;

        size_t GetSectorSize() override;

    private:
        uint32_t m_ID;
        NVMeController* m_controller;
        NVMeIOQueue* m_IOQueue;
        size_t m_MaxTransferSize;
        size_t m_SectorSize;
        IdentifyNamespace* m_info;
    };

}

#endif /* _HAL_NVME_HPP */