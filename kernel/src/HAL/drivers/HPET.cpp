/*
Copyright (©) 2024  Frosty515

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

#include "HPET.hpp"

#include <assert.h>
#include <stdio.h>
#include <util.h>

#include <arch/x86_64/interrupts/IRQ.hpp>
#include <arch/x86_64/interrupts/APIC/IOAPIC.hpp>
#include <arch/x86_64/io.h>

#define HPET_INTHandler(x) void HPET_INT##x() { if (g_HPET != nullptr) g_HPET->HandleInterrupt(x); }

HPET_INTHandler(0)
HPET_INTHandler(1)
HPET_INTHandler(2)
HPET_INTHandler(3)
HPET_INTHandler(4)
HPET_INTHandler(5)
HPET_INTHandler(6)
HPET_INTHandler(7)
HPET_INTHandler(8)
HPET_INTHandler(9)
HPET_INTHandler(10)
HPET_INTHandler(11)
HPET_INTHandler(12)
HPET_INTHandler(13)
HPET_INTHandler(14)
HPET_INTHandler(15)
HPET_INTHandler(16)
HPET_INTHandler(17)
HPET_INTHandler(18)
HPET_INTHandler(19)
HPET_INTHandler(20)
HPET_INTHandler(21)
HPET_INTHandler(22)
HPET_INTHandler(23)
HPET_INTHandler(24)
HPET_INTHandler(25)
HPET_INTHandler(26)
HPET_INTHandler(27)
HPET_INTHandler(28)
HPET_INTHandler(29)
HPET_INTHandler(30)
HPET_INTHandler(31)

#undef HPET_INTHandler

void* HPET_GetINTHandler(uint8_t index) {
    switch (index) {
#define HPET_INTSWICH(x) case x: return (void*)&HPET_INT##x;
        HPET_INTSWICH(0)
        HPET_INTSWICH(1)
        HPET_INTSWICH(2)
        HPET_INTSWICH(3)
        HPET_INTSWICH(4)
        HPET_INTSWICH(5)
        HPET_INTSWICH(6)
        HPET_INTSWICH(7)
        HPET_INTSWICH(8)
        HPET_INTSWICH(9)
        HPET_INTSWICH(10)
        HPET_INTSWICH(11)
        HPET_INTSWICH(12)
        HPET_INTSWICH(13)
        HPET_INTSWICH(14)
        HPET_INTSWICH(15)
        HPET_INTSWICH(16)
        HPET_INTSWICH(17)
        HPET_INTSWICH(18)
        HPET_INTSWICH(19)
        HPET_INTSWICH(20)
        HPET_INTSWICH(21)
        HPET_INTSWICH(22)
        HPET_INTSWICH(23)
        HPET_INTSWICH(24)
        HPET_INTSWICH(25)
        HPET_INTSWICH(26)
        HPET_INTSWICH(27)
        HPET_INTSWICH(28)
        HPET_INTSWICH(29)
        HPET_INTSWICH(30)
        HPET_INTSWICH(31)
#undef HPET_INTSWICH
    default:
        return nullptr;
    }
}


HPET::HPET() : m_regs(nullptr) {

}

HPET::~HPET() {

}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"

void __attribute__((no_sanitize("undefined"))) HPET::Init(HPETRegisters* regs) {
    m_regs = regs;
    uint64_t GeneralCAPID = volatile_read64(regs->GeneralCAPID);
    uint32_t CounterClockPeriod = GeneralCAPID >> 32;
    uint16_t VendorID = (GeneralCAPID >> 16) & 0xFFFF;
    bool LegacyReplacementSupport = (GeneralCAPID & (1 << 13)) >> 13;
    uint8_t MaxTimers = (GeneralCAPID & (0x1F << 8)) >> 8;
    bool CounterSize = (GeneralCAPID & (1 << 13)) >> 13;
    
    assert(LegacyReplacementSupport && CounterSize); // don't have the support yet to choose an INT line or 32-bit timers.

    uint64_t GeneralConfig = volatile_read64(regs->GeneralConfig);
    GeneralConfig &= ~3; // Disable the timer
    //GeneralConfig |= 2; // Legacy Replacement

    volatile_write64(regs->MainCounterValue, 0);

    for (uint8_t i = 0; i < 32; i++) {
        m_timers[i].active = false;
        m_timers[i].usable = false;
        m_timers[i].callback = nullptr;
        m_timers[i].data = nullptr;
    }

    for (uint8_t i = 0; i <= MaxTimers; i++) {
        HPETTimerRegisters* timer = &regs->Timer[i];
        uint64_t ConfigCAP = volatile_read64(timer->ConfigCAP);
        ConfigCAP &= ~(0x7F0E); // Clear INTType, INTEnable, Type, Mode32, INTRoute, FSB
        volatile_write64(timer->ConfigCAP, ConfigCAP);
        m_timers[i].active = false;
        m_timers[i].usable = true;
        m_timers[i].callback = nullptr;
        m_timers[i].data = nullptr;
        m_timers[i].IRQ = INVALID_IRQ;
    }


    GeneralConfig |= 1; // Enable the timers
    volatile_write64(regs->GeneralConfig, GeneralConfig);

    m_VendorID = VendorID;
    m_CounterClockPeriod = CounterClockPeriod;
}


bool HPET::StartTimer(uint64_t femtoSec, HPETCallback callback, void* data) {
    uint8_t timer = 0;
    bool found = false;
    for (uint8_t i = 0; i < 32; i++) {
        if (m_timers[i].usable && !m_timers[i].active) {
            timer = i;
            found = true;
            break;
        }
    }
    if (!found)
        return false;

    m_timers[timer].active = true;
    m_timers[timer].callback = callback;
    m_timers[timer].data = data;
    m_timers[timer].femtoSec = femtoSec;

#ifdef __x86_64__
    if (m_timers[timer].IRQ == INVALID_IRQ) {
        uint32_t IRQCap = volatile_read64(m_regs->Timer[timer].ConfigCAP) >> 32;
        uint8_t IRQ = x86_64_IRQ_AllocateIRQ(IRQCap, 0);
        if (IRQ == INVALID_IRQ) {
            m_timers[timer].usable = false;
            m_timers[timer].active = false;
            dbgprintf("HPET: Failed to find an IRQ for timer %hhu\n", timer);
            return StartTimer(femtoSec, callback, data); // try again with a different timer
        }
        m_timers[timer].IRQ = IRQ;
        x86_64_IRQ_RegisterHandler(m_timers[timer].IRQ, (x86_64_IRQHandler_t)HPET_GetINTHandler(timer));
    }
#endif

    uint8_t IRQ = m_timers[timer].IRQ;

    HPETTimerRegisters* timerRegs = &m_regs->Timer[timer];
    uint64_t ConfigCAP = volatile_read64(timerRegs->ConfigCAP);
    ConfigCAP &= ~(0x7F0E); // Clear INTType, INTEnable, Type, Mode32, INTRoute, FSB
    ConfigCAP |= (1 << 2); // Enable Interrupts
    ConfigCAP |= ((uint32_t)IRQ << 9);

    // Calculate the comparator value
    uint64_t ticks = femtoSec / m_CounterClockPeriod;
    volatile_write64(timerRegs->ConfigCAP, ConfigCAP);
    volatile_write64(timerRegs->ComparatorValue, volatile_read64(m_regs->MainCounterValue) + ticks);

#ifdef __x86_64__
    {
        x86_64_IOAPIC* ioapic = x86_64_IOAPIC_GetIOAPICForIRQ(IRQ);
        x86_64_IOAPIC_RedirectionEntry entry = ioapic->GetRedirectionEntry(IRQ - ioapic->GetIRQBase());
        entry.Mask = 0;
        ioapic->SetRedirectionEntry(IRQ - ioapic->GetIRQBase(), entry);
    }
#endif

    return true;
}

uint64_t HPET::GetMainCounter() const {
    return volatile_read64(m_regs->MainCounterValue);
}

uint64_t* HPET::GetMainCounterAddress() const {
    return &m_regs->MainCounterValue;
}

#pragma GCC diagnostic pop

uint64_t HPET::GetClockPeriod() const {
    return m_CounterClockPeriod;
}

void HPET::HandleInterrupt(uint8_t timer) {
    m_timers[timer].active = false;
    m_timers[timer].callback(m_timers[timer].data);
}

const char* HPET::getVendorName() {
    switch (m_VendorID) {
    case 0x8086:
        return "Intel";
    default:
        return "Unknown";
    }
}

const char* HPET::getDeviceName() {
    return "HPET";
}

HPET* g_HPET = nullptr;
