#ifndef _HAL_ACPI_XSDT_HPP
#define _HAL_ACPI_XSDT_HPP

#include <stdint.h>

#include "SDT.hpp"

bool InitAndValidateXSDT(void* XSDT);

ACPISDTHeader* getOtherSDT(uint64_t index);

#endif /* _HAL_ACPI_XSDT_HPP */