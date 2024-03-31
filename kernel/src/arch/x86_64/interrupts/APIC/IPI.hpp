#ifndef _X86_64_APIC_IPI_HPP
#define _X86_64_APIC_IPI_HPP

#include <stdint.h>

#include "LocalAPIC.hpp"

enum class x86_64_IPI_DeliveryMode {
    Fixed = 0,
    LowPriority = 1,
    SMI = 2,
    NMI = 4,
    INIT = 5,
    StartUp = 6
};

void x86_64_SendIPI(x86_64_LocalAPICRegisters* regs, uint8_t vector, x86_64_IPI_DeliveryMode deliveryMode, bool level, bool trigger_mode, uint8_t destinationType, uint8_t destination);

#endif /* _X86_64_APIC_IPI_HPP */