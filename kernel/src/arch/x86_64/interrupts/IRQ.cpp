/*
Copyright (©) 2022-2024  Frosty515

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "IRQ.hpp"
#include "pic.hpp"

#include "APIC/IOAPIC.hpp"

#include "../io.h"

#include <stdio.h>

#include <Data-structures/Bitmap.hpp>

#define PIC_REMAP_OFFSET 0x20

uint8_t g_IOAPIC_INT_END = 0;

x86_64_IRQHandler_t* g_IRQHandlers;
uint8_t g_IRQHandlersCount = 0;

Bitmap g_IRQBitmap;
spinlock_new(g_IRQBitmapLock);

void x86_64_PIC_IRQ_Handler(x86_64_Interrupt_Registers* regs) {
    uint8_t irq = regs->interrupt - PIC_REMAP_OFFSET;
    x86_64_PIC_sendEOI(irq); // ignore any interrupts that aren't routed through the I/O APIC(s)
}

void x86_64_IRQ_Handler(x86_64_Interrupt_Registers* regs) {
    uint8_t IRQ = regs->interrupt - PIC_REMAP_OFFSET - 0x10;

    if (g_IRQHandlers[IRQ] != nullptr)
        g_IRQHandlers[IRQ](regs);
    else
        dbgprintf("Unhandled IRQ: %#.2hhx\n", IRQ);

    x86_64_IOAPIC_SendEOI();
}

// we have to configure and disable the PIC in preparation for the I/O APIC
void x86_64_IRQ_EarlyInit() {
    x86_64_PIC_Configure(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET + 8, false);
    x86_64_PIC_Disable();

    for (uint8_t i = 0; i < 16; i++)
        x86_64_ISR_RegisterHandler(i + PIC_REMAP_OFFSET, x86_64_PIC_IRQ_Handler);
}

void x86_64_IRQ_FullInit() {
    uint64_t INTMax = 0;
    for (uint64_t i = 0; i < g_IOAPICs.getCount(); i++) {
        x86_64_IOAPIC* ioapic = g_IOAPICs.get(i);
        for (uint64_t j = ioapic->GetIRQBase(); j < ioapic->GetIRQEnd(); j++)
            INTMax = j;
    }
    /*
    0x00-0x1F - reserved for exceptions
    0x20-0x2F - reserved for the legacy 8259 PICs
    0x30-0xEF - available for I/O APICs
    0xF0-0xFF - reserved for LAPIC stuff or IPIs
    */
    assert(INTMax < (0x100 - 0x40)); // too many interrupts. 
    g_IOAPIC_INT_END = INTMax + PIC_REMAP_OFFSET + 0x10;
    g_IRQHandlersCount = INTMax;
    g_IRQHandlers = new x86_64_IRQHandler_t[g_IRQHandlersCount];
    for (uint64_t i = 0; i < g_IRQHandlersCount; i++)
        g_IRQHandlers[i] = nullptr;

    uint64_t k = PIC_REMAP_OFFSET + 0x10;
    for (uint64_t i = 0; i < g_IOAPICs.getCount(); i++) {
        x86_64_IOAPIC* ioapic = g_IOAPICs.get(i);
        ioapic->SetINTStart(k);
        for (uint64_t j = ioapic->GetIRQBase(); j < ioapic->GetIRQEnd(); j++, k++)
            x86_64_ISR_RegisterHandler(k, x86_64_IRQ_Handler);
    }
    uint8_t* buffer = new uint8_t[g_IRQHandlersCount / 8];
    memset(buffer, 0, g_IRQHandlersCount / 8);
    g_IRQBitmap.SetBuffer(buffer);
    g_IRQBitmap.SetSize(g_IRQHandlersCount / 8);
    // 8042 PS/2 controller device IRQ mappings
    x86_64_IRQ_ReserveIRQ(1); // keyboard
    x86_64_IRQ_ReserveIRQ(12); // mouse
}

void x86_64_IRQ_ReserveIRQ(uint8_t irq) {
    spinlock_acquire(&g_IRQBitmapLock);
    g_IRQBitmap.Set(irq, true);
    spinlock_release(&g_IRQBitmapLock);
}

void x86_64_IRQ_UnreserveIRQ(uint8_t irq) {
    spinlock_acquire(&g_IRQBitmapLock);
    g_IRQBitmap.Set(irq, false);
    spinlock_release(&g_IRQBitmapLock);
}

bool x86_64_IRQ_IsIRQReserved(uint8_t irq) {
    spinlock_acquire(&g_IRQBitmapLock);
    bool ret = g_IRQBitmap[irq];
    spinlock_release(&g_IRQBitmapLock);
    return ret;
}

uint8_t x86_64_IRQ_AllocateIRQ() {
    spinlock_acquire(&g_IRQBitmapLock);
    uint8_t IRQ = INVALID_IRQ;
    for (IRQ = 0; IRQ < g_IRQHandlersCount; IRQ++) {
        if (!g_IRQBitmap[IRQ]) {
            g_IRQBitmap.Set(IRQ, true);
            break;
        }
    }
    spinlock_release(&g_IRQBitmapLock);
    return IRQ;
}

uint8_t x86_64_IRQ_AllocateIRQ(uint64_t mask, uint64_t shift) {
    spinlock_acquire(&g_IRQBitmapLock);
    uint8_t IRQ = INVALID_IRQ;
    for (uint8_t i = 0; i < 64; i++) {
        if ((mask >> i) & 1) {
            IRQ = i + shift;
            if (!g_IRQBitmap[IRQ]) {
                g_IRQBitmap.Set(IRQ, true);
                break;
            }
        }
    }
    spinlock_release(&g_IRQBitmapLock);
    return IRQ;
}

void x86_64_IRQ_FreeIRQ(uint8_t irq) {
    spinlock_acquire(&g_IRQBitmapLock);
    g_IRQBitmap.Set(irq, false);
    spinlock_release(&g_IRQBitmapLock);
}

void x86_64_IRQ_RegisterHandler(uint8_t irq, x86_64_IRQHandler_t handler) {
    if (irq < g_IRQHandlersCount)
        g_IRQHandlers[irq] = handler;
    else
        dbgprintf("Tried to register IRQ handler for invalid IRQ: %#.2hhx\n", irq);
}
