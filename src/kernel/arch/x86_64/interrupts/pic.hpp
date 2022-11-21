#ifndef _KERNEL_X86_64_PIC_HPP
#define _KERNEL_X86_64_PIC_HPP

#include <stdint.h>
#include <stddef.h>

void x86_64_PIC_SetMask(uint16_t newMask);
uint16_t x86_64_PIC_GetMask();
void x86_64_PIC_sendEOI(uint8_t irq);
void x86_64_PIC_Disable();
void x86_64_PIC_Mask(uint8_t irq);
void x86_64_PIC_Unmask(uint8_t irq);
void x86_64_PIC_Configure(uint8_t offset_PIC1, uint8_t offset_PIC2, bool autoEOI);
uint16_t x86_64_PIC_ReadIRQRequestRegister();
uint16_t x86_64_PIC_ReadInServiceRegister();
bool x86_64_PIC_Probe();

#endif /* _KERNEL_X86_64_PIC_HPP */