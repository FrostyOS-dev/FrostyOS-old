#include "IPI.hpp"

#include <stdio.h>

void x86_64_SendIPI(x86_64_LocalAPICRegisters* regs, uint8_t vector, x86_64_IPI_DeliveryMode deliveryMode, bool level, bool trigger_mode, uint8_t destinationType, uint8_t destination) {
    uint8_t i_deliveryMode = 0;
    
    switch (deliveryMode) {
        case x86_64_IPI_DeliveryMode::Fixed:
            i_deliveryMode = 0;
            break;
        case x86_64_IPI_DeliveryMode::LowPriority:
            i_deliveryMode = 1;
            break;
        case x86_64_IPI_DeliveryMode::SMI:
            i_deliveryMode = 2;
            break;
        case x86_64_IPI_DeliveryMode::NMI:
            i_deliveryMode = 4;
            break;
        case x86_64_IPI_DeliveryMode::INIT:
            i_deliveryMode = 5;
            break;
        case x86_64_IPI_DeliveryMode::StartUp:
            i_deliveryMode = 6;
            break;
        default:
            return;
    }
    
    uint32_t ICR0 = *(volatile uint32_t*)(&regs->ICR0);
    uint32_t ICR1 = *(volatile uint32_t*)(&regs->ICR1);

    ICR0 &= 0xFFF32000;
    ICR0 |= vector;
    ICR0 |= (uint32_t)(i_deliveryMode & 0b111) << 8;
    ICR0 |= (level ? 1 : 0) << 14;
    ICR0 |= (trigger_mode ? 1 : 0) << 15;
    ICR0 |= (uint32_t)(destinationType & 0b11) << 18;

    ICR1 &= 0x00FFFFFF;
    ICR1 |= destination << 24;

    //dbgprintf("ICR0 = %#.8x, ICR1 = %#.8x, deliveryMode = %x\n", ICR0, ICR1, i_deliveryMode);

    *(volatile uint32_t*)(&regs->ICR1) = ICR1; // must write to ICR1 first
    *(volatile uint32_t*)(&regs->ICR0) = ICR0;

    while(*(volatile uint32_t*)(&regs->ICR0) & (1 << 12)) { __asm__ volatile ("" ::: "memory"); } // wait for IPI to be sent
}