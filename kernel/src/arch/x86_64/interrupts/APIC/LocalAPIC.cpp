#include "LocalAPIC.hpp"
#include "IPI.hpp"

#include <string.h>
#include <util.h>

#include "../../io.h"
#include "../../Processor.hpp"
#include "../../PIT.hpp"

#include "../../Memory/PageMapIndexer.hpp"

#include <Memory/PageManager.hpp>

#include <HAL/hal.hpp>
#include <HAL/time.h>
#include <HAL/drivers/HPET.hpp>

#include "../pic.hpp"


extern void* ap_trampoline;

extern "C" {
uint64_t aps_running;
}

x86_64_LocalAPIC::x86_64_LocalAPIC(void* baseAddress, bool BSP, uint8_t ID) : m_registers((x86_64_LocalAPICRegisters*)baseAddress), m_BSP(BSP), m_ID(ID), m_timerLock(0) {

}

void x86_64_LocalAPIC::SendEOI() {
    volatile_write32(m_registers->EOI, 0);
}

void x86_64_LocalAPIC::StartCPU() {
    if (!m_BSP) {
        x86_64_DisableInterrupts();
        //x86_64_PIC_Disable();
        x86_64_map_page(&K_PML4_Array, (void*)0x0000, (void*)0x0000, 0x3); // Present, Read/Write, Execute
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
        x86_64_remap_page(&K_PML4_Array, m_registers, 0x8000013); // Present, Read/Write, No execute, Cache disable
        //x86_64_unmap_page(&K_PML4_Array, (void*)0x0000);
        uint64_t old_aps_running = aps_running;
        m_registers->ErrorStatus = 0; // clear errors
        x86_64_SendIPI(m_registers, 0, x86_64_IPI_DeliveryMode::INIT, false, false, 0, m_ID); // send INIT IPI
        x86_64_SendIPI(m_registers, 0, x86_64_IPI_DeliveryMode::INIT, true, false, 0, m_ID); // deassert
        for (int i = 0; i < 2; i++) {
            m_registers->ErrorStatus = 0; // clear errors
            x86_64_SendIPI(m_registers, 0x00, x86_64_IPI_DeliveryMode::StartUp, false, false, 0, m_ID); // send SIPI
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
        *jump_addr = (uint64_t)(void*)&Processor::Init;
        *jmp_arg = (uint64_t)proc;
        *stack = (uint64_t)g_KPM->AllocatePages(KERNEL_STACK_SIZE / PAGE_SIZE);
        *kernel_stack_offset = Processor::get_kernel_stack_offset();
        *start_lock = 0;
    }
}

void x86_64_LocalAPIC::Init() {
    if (!m_BSP) // unmap first page
        x86_64_unmap_page(&K_PML4_Array, (void*)0);
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
    lvt_timer &= 0xFFFCFF00; // clear vector and mode
    lvt_timer |= 0x20000; // periodic mode
    volatile_write32(m_registers->LVT_TIMER, lvt_timer);

    // divide by 16
    volatile_write32(m_registers->DivideConfiguration, 0x3);

    // we wait for 1ms of the HAL timer
    while(!g_HPET->StartTimer(1'000'000'000'000, (HPETCallback)&x86_64_LocalAPIC::LAPICTimerCallback, this)) {
        dbgprintf("Warning: HPET timer failed to start for LAPIC timer init on CPU %hu\n", m_ID);
        sleep(1);
    }
    volatile_write32(m_registers->InitialCount, -1);
    __atomic_store_8(&m_timerComplete, 1, __ATOMIC_SEQ_CST);
    x86_64_EnableInterrupts();
    while (__atomic_load_8(&m_timerComplete, __ATOMIC_SEQ_CST) != 0)
        __atomic_signal_fence(__ATOMIC_SEQ_CST);
}

void x86_64_LocalAPIC::AllowInitTimer() {
    spinlock_release(&(this->m_timerLock));
}

uint8_t x86_64_LocalAPIC::GetID() const {
    return m_ID;
}

void x86_64_LocalAPIC::LAPICTimerCallback() {
    uint32_t ticksIn1ms = (0xFFFFFFFF - volatile_read32(m_registers->CurrentCount))*16;
    dbgprintf("LAPIC %hu timer ticked %u ticks in 1ms.\n", m_ID, ticksIn1ms);
    __asm__ volatile ("" ::: "memory");
    __atomic_clear(&m_timerComplete, __ATOMIC_SEQ_CST);
}
