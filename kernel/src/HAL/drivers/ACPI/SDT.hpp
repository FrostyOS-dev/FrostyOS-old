#ifndef _HAL_ACPI_SDT_HPP
#define _HAL_ACPI_SDT_HPP

#include <stdint.h>

struct ACPISDTHeader {
  char Signature[4];
  uint32_t Length;
  uint8_t Revision;
  uint8_t Checksum;
  char OEMID[6];
  char OEMTableID[8];
  uint32_t OEMRevision;
  uint32_t CreatorID;
  uint32_t CreatorRevision;
};

bool doChecksum(ACPISDTHeader* h);

#endif /* _HAL_ACPI_SDT_HPP */