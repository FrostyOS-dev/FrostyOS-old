#ifndef _HAL_ACPI_HPET_HPP
#define _HAL_ACPI_HPET_HPP

#include "SDT.hpp"

struct HPETACPIHeader {
    uint8_t HardwareRevID;
    uint8_t ComparatorCount : 5;
    uint8_t CounterSize : 1;
    uint8_t Reserved : 1;
    uint8_t LegacyReplacement : 1;
    uint16_t PCIVendorID;
    uint8_t AddressSpaceID; // 0x00 = Memory, 0x01 = I/O
    uint8_t RegisterBitWidth;
    uint8_t RegisterBitOffset;
    uint8_t Reserved2;
    uint64_t Address;
    uint8_t HPETNumber;
    uint16_t MinimumTickPeriodic;
    uint8_t PageProtection : 4;
    uint8_t OEMSpecific : 4;
} __attribute__((packed));

bool InitAndValidateHPET(ACPISDTHeader* HPET);

void* GetHPETAddress();

#endif /* _HAL_ACPI_HPET_HPP */