#ifndef _HAL_ACPI_MCFG_HPP
#define _HAL_ACPI_MCFG_HPP

#include "SDT.hpp"

struct MCFGEntry {
    uint64_t Address;
    uint16_t SegmentGroupNumber;
    uint8_t StartBusNumber;
    uint8_t EndBusNumber;
    uint32_t Reserved;
} __attribute__((packed));

bool InitAndValidateMCFG(ACPISDTHeader* MCFG);

MCFGEntry* GetMCFGEntry(uint64_t index);

uint64_t GetMCFGEntryCount();

#endif /* _HAL_ACPI_MCFG_HPP */