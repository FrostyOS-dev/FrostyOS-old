#ifndef _HAL_ACPI_RSDP_HPP
#define _HAL_ACPI_RSDP_HPP

#include <stdint.h>

struct RSDPDescriptor {
 char Signature[8];
 uint8_t Checksum;
 char OEMID[6];
 uint8_t Revision;
 uint32_t RSDTAddress;
} __attribute__ ((packed));

struct RSDPDescriptor20 {
 RSDPDescriptor firstPart;
 
 uint32_t Length;
 uint64_t XSDTAddress;
 uint8_t ExtendedChecksum;
 uint8_t reserved[3];
} __attribute__ ((packed));

bool InitAndValidateRSDP(void* RSDP);

bool IsXSDTAvailable();

void* GetXSDT();

#endif /* _HAL_ACPI_RSDP_HPP */