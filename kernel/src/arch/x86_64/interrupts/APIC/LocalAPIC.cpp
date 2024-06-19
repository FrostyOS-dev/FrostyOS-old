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

#include "LocalAPIC.hpp"
#include "IPI.hpp"

#include "../../io.h"
#include "../../Processor.hpp"

#include "../../Memory/PageMapIndexer.hpp"
#include "../../Memory/PAT.hpp"

#include "../../Scheduling/taskutil.hpp"

#include <math.h>
#include <string.h>
#include <util.h>

#include <Memory/PageManager.hpp>
#include <Memory/Stack.hpp>

#include <HAL/hal.hpp>
#include <HAL/time.h>
#include <HAL/drivers/HPET.hpp>

#include <Scheduling/Scheduler.hpp>


extern void* ap_trampoline;

extern "C" {
uint64_t aps_running;
}

void x86_64_LAPIC_TimerCallback(x86_64_Interrupt_Registers* regs) {
    x86_64_GetCurrentLocalAPIC()->LAPICTimerCallback(regs);
}

x86_64_LocalAPIC::x86_64_LocalAPIC(void* baseAddress, bool BSP, uint8_t ID) : m_registers((x86_64_LocalAPICRegisters*)baseAddress), m_BSP(BSP), m_ID(ID), m_timerLock(0) {

}

void InitProcessor(void* obj) {
    Processor* proc = (Processor*)obj;
    proc->Init(nullptr, 0, 0, 0, 0, 0, FrameBuffer());
}

void x86_64_LocalAPIC::SendEOI() {
    volatile_write32(m_registers->EOI, 0);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"

void x86_64_LocalAPIC::StartCPU() {
    if (!m_BSP) {
        x86_64_DisableInterrupts();
        //x86_64_PIC_Disable();
        // copy the ap trampoline to 0x0000
        memcpy((void*)0x0000, &ap_trampoline, 0x1000);
        uint32_t* CR3_value = (uint32_t*)0xFFC;
        uint32_t* start_lock = (uint32_t*)0xFF8;
        uint64_t* jump_addr = (uint64_t*)0xFF0;
        uint64_t* jmp_arg = (uint64_t*)0xFE8;
        uint64_t* stack = (uint64_t*)0xFE0;
        uint64_t* kernel_stack_offset = (uint64_t*)0xFD8;
        if ((uint64_t)g_KPML4_physical >= GiB(4)) {
            PANIC("Kernel PML4 array is not in the first 4GiB of memory");
        }
        *CR3_value = (uint32_t)(uint64_t)x86_64_get_physaddr(&K_PML4_Array, &K_PML4_Array);
        *start_lock = 1; // we prevent it from starting executing actual kernel code until we are ready
        spinlock_acquire(&m_timerLock); // we can't have the APIC timer being configured until after the HPET is initialized.
        x86_64_remap_page(&K_PML4_Array, m_registers, 0x8000003 | x86_64_GetPTEFlagsFromPAT(x86_64_PATType::Uncacheable)); // Present, Read/Write, No execute, Cache disable
        //x86_64_unmap_page(&K_PML4_Array, (void*)0x0000);
        uint64_t old_aps_running = aps_running;
        m_registers->ErrorStatus = 0; // clear errors
        x86_64_SendIPI(m_registers, 0, x86_64_IPI_DeliveryMode::INIT, false, false, x86_64_IPI_DestinationShorthand::NoShorthand, m_ID); // send INIT IPI
        x86_64_SendIPI(m_registers, 0, x86_64_IPI_DeliveryMode::INIT, true, false, x86_64_IPI_DestinationShorthand::NoShorthand, m_ID); // deassert
        for (int i = 0; i < 2; i++) {
            m_registers->ErrorStatus = 0; // clear errors
            x86_64_SendIPI(m_registers, 0x00, x86_64_IPI_DeliveryMode::StartUp, false, false, x86_64_IPI_DestinationShorthand::NoShorthand, m_ID); // send SIPI
            uint64_t current_time = GetTimer();
            while (aps_running == old_aps_running && (GetTimer() - current_time) < (i == 0 ? 1 : 1000)) { // timeout of 1ms for first attempt, 1000ms for second
                __asm__ volatile("" ::: "memory");
            }
            if (aps_running != old_aps_running)
                break;
            else if (i == 1) {
                PANIC("AP did not start");
            }
        }
        Processor* proc = new Processor(false);
        proc->SetLocalAPIC(this);
        *jump_addr = (uint64_t)InitProcessor;
        *jmp_arg = (uint64_t)proc;
        *stack = (uint64_t)CreateKernelStack();
        *kernel_stack_offset = Processor::get_kernel_stack_offset();
        *start_lock = 0;
        x86_64_EnableInterrupts();
    }
}

#pragma GCC diagnostic pop

void x86_64_LocalAPIC::Init() {
    // we mask all LVTs
    uint8_t max_lvt = (volatile_read32(m_registers->Version) >> 16) & 0xFF;
    for (uint64_t i = (uint64_t)m_registers + 0x320; i < ((uint64_t)m_registers + 0x320 + max_lvt * 0x10); i += 0x10) {
        uint32_t lvt = *(volatile uint32_t*)i;
        lvt |= 1 << 16; // mask
        *(volatile uint32_t*)i = lvt;
    }

    // we set the spurious interrupt vector to 0xFF and enable the APIC.
    uint32_t spurious = volatile_read32(m_registers->SpuriousInterruptVector);
    spurious &= 0xFFFEFF00; // clear APIC software enable, spurious vector
    spurious |= 0x100; // enable APIC
    spurious |= 0xFF; // spurious vector
    volatile_write32(m_registers->SpuriousInterruptVector, spurious);

    uint32_t TPR = volatile_read32(m_registers->TaskPriority);
    // set the TPR so that all interrupts are accepted
    TPR &= 0xFFFFFF00;
    volatile_write32(m_registers->TaskPriority, TPR);
}

void x86_64_LocalAPIC::InitTimer() {
    spinlock_acquire(&(this->m_timerLock)); // we must wait until the HPET is ready

    // we set the APIC timer to periodic mode
    uint32_t lvt_timer = volatile_read32(m_registers->LVT_TIMER);
    lvt_timer &= 0xFFF9FF00; // clear vector and mode (will run in one-shot mode)
    volatile_write32(m_registers->LVT_TIMER, lvt_timer);

    // divide by 16
    volatile_write32(m_registers->DivideConfiguration, 0x3);

    // we poll for 10ms to pass on the HPET, and see how many LAPIC ticks have occurred.

    uint64_t msInHPETTicks = 10'000'000'000'000 / g_HPET->GetClockPeriod();

    uint64_t start = g_HPET->GetMainCounter();
    volatile_write32(m_registers->InitialCount, 0xFFFFFFFF);
    __atomic_signal_fence(__ATOMIC_SEQ_CST);
    while (true) {
        uint64_t current = g_HPET->GetMainCounter();
        if ((current - start) >= msInHPETTicks)
            break;
        __atomic_signal_fence(__ATOMIC_SEQ_CST);
    }
    uint64_t ticksIn1ms = (0xFFFFFFFF - volatile_read32(m_registers->CurrentCount)) * 16;

    // stop the timer for now
    volatile_write32(m_registers->InitialCount, 0);
    
    uint64_t freq = ticksIn1ms * 100;
    // align freq to nearest 100kHz
    freq = (freq + 50'000) / 100'000 * 100'000;
    m_timer_base_freq = freq;
    uint64_t divisor = m_timer_base_freq / 1'000; // 1kHz
    uint8_t real_divisor_shift = min(get_lsb(divisor), 7);
    uint64_t real_divisor = divisor >> real_divisor_shift;
    m_timer_current_freq = m_timer_base_freq / real_divisor;
    m_timer_rticks_per_tick = real_divisor;

    // now we set the divide configuration using this table:
    /*
        000: Divide by 2
        001: Divide by 4
        010: Divide by 8
        011: Divide by 16
        100: Divide by 32
        101: Divide by 64
        110: Divide by 128
        111: Divide by 1
    */
    if (real_divisor_shift == 0)
        volatile_write32(m_registers->DivideConfiguration, 0b111);
    else
        volatile_write32(m_registers->DivideConfiguration, real_divisor_shift - 1);

    if (m_BSP)
        x86_64_ISR_RegisterHandler(LAPIC_TIMER_INT, x86_64_LAPIC_TimerCallback);

    // we set the timer to periodic mode

    lvt_timer = volatile_read32(m_registers->LVT_TIMER);
    lvt_timer &= 0xFFF8FF00; // clear vector, mode, and mask
    lvt_timer |= 0x20000 | LAPIC_TIMER_INT; // set periodic mode and vector
    volatile_write32(m_registers->LVT_TIMER, lvt_timer);

    // set the initial count

    volatile_write32(m_registers->InitialCount, m_timer_rticks_per_tick);
}

void x86_64_LocalAPIC::AllowInitTimer() {
    spinlock_release(&(this->m_timerLock));
}

uint8_t x86_64_LocalAPIC::GetID() const {
    return m_ID;
}

void x86_64_LocalAPIC::LAPICTimerCallback(x86_64_Interrupt_Registers* regs) {
    if (!(Scheduling::Scheduler::isRunning() && Scheduling::Scheduler::GlobalIsRunning())) {
        SendEOI();
        return;
    }
    x86_64_SaveIRegistersToThread(Scheduling::Scheduler::GetCurrent(), regs);
    Scheduling::Scheduler::TimerTick(regs);
    SendEOI();
}

x86_64_LocalAPIC* x86_64_GetCurrentLocalAPIC() {
    return GetCurrentProcessor()->GetLocalAPIC();
}
