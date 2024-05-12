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

#ifndef _X86_64_LOCAL_APIC_HPP
#define _X86_64_LOCAL_APIC_HPP

#include <stdint.h>
#include <spinlock.h>

#include "../isr.hpp"

#include <Data-structures/LinkedList.hpp>

#define LAPIC_TIMER_INT 0xF0

struct x86_64_LocalAPICRegisters {
#define LAPIC_REGISTER(name) uint32_t name; uint32_t _align_##name[3]
    uint32_t Reserved0[8];
    LAPIC_REGISTER(ID);
    LAPIC_REGISTER(Version);
    uint32_t Reserved1[16];
    LAPIC_REGISTER(TaskPriority);
    LAPIC_REGISTER(ArbitrationPriority);
    LAPIC_REGISTER(ProcessorPriority);
    LAPIC_REGISTER(EOI);
    LAPIC_REGISTER(RemoteRead);
    LAPIC_REGISTER(LogicalDestination);
    LAPIC_REGISTER(DestinationFormat);
    LAPIC_REGISTER(SpuriousInterruptVector);
    LAPIC_REGISTER(ISR0);
    LAPIC_REGISTER(ISR1);
    LAPIC_REGISTER(ISR2);
    LAPIC_REGISTER(ISR3);
    LAPIC_REGISTER(ISR4);
    LAPIC_REGISTER(ISR5);
    LAPIC_REGISTER(ISR6);
    LAPIC_REGISTER(ISR7);
    LAPIC_REGISTER(TMR0);
    LAPIC_REGISTER(TMR1);
    LAPIC_REGISTER(TMR2);
    LAPIC_REGISTER(TMR3);
    LAPIC_REGISTER(TMR4);
    LAPIC_REGISTER(TMR5);
    LAPIC_REGISTER(TMR6);
    LAPIC_REGISTER(TMR7);
    LAPIC_REGISTER(IRR0);
    LAPIC_REGISTER(IRR1);
    LAPIC_REGISTER(IRR2);
    LAPIC_REGISTER(IRR3);
    LAPIC_REGISTER(IRR4);
    LAPIC_REGISTER(IRR5);
    LAPIC_REGISTER(IRR6);
    LAPIC_REGISTER(IRR7);
    LAPIC_REGISTER(ErrorStatus);
    uint32_t Reserved2[24];
    LAPIC_REGISTER(LVT_CMCI);
    LAPIC_REGISTER(ICR0);
    LAPIC_REGISTER(ICR1);
    LAPIC_REGISTER(LVT_TIMER);
    LAPIC_REGISTER(LVT_THERMAL);
    LAPIC_REGISTER(LVT_PERFORMANCE_MONITORING_COUNTERS);
    LAPIC_REGISTER(LVT_LINT0);
    LAPIC_REGISTER(LVT_LINT1);
    LAPIC_REGISTER(LVT_ERROR);
    LAPIC_REGISTER(InitialCount);
    LAPIC_REGISTER(CurrentCount);
    uint32_t Reserved3[16];
    LAPIC_REGISTER(DivideConfiguration);
    uint32_t Reserved4[4];
#undef LAPIC_REGISTER
} __attribute__((packed));

void x86_64_LAPIC_TimerCallback(x86_64_Interrupt_Registers* regs);

class x86_64_LocalAPIC {
public:
    x86_64_LocalAPIC(void* baseAddress, bool BSP, uint8_t ID);

    void SendEOI();

    void StartCPU();

    void Init();

    void InitTimer();
    void AllowInitTimer();

    uint8_t GetID() const;

    void LAPICTimerCallback(x86_64_Interrupt_Registers* regs);

    x86_64_LocalAPICRegisters* GetRegisters() {
        return m_registers;
    }

private:
    x86_64_LocalAPICRegisters* m_registers;
    bool m_BSP;
    uint8_t m_ID;
    spinlock_t m_timerLock;
    
    uint64_t m_timer_base_freq;
    uint64_t m_timer_current_freq;
    uint64_t m_timer_rticks_per_tick;
    uint64_t m_timer_current_count; // just used to divide correctly
};

x86_64_LocalAPIC* x86_64_GetCurrentLocalAPIC();

#endif /* _X86_64_LOCAL_APIC_HPP */