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

#include "Processor.hpp"
#include "Stack.hpp"
#include "cpuid.hpp"

#include "interrupts/IDT.hpp"
#include "interrupts/isr.hpp"
#include "interrupts/IRQ.hpp"
#include "interrupts/NMI.hpp"
#include "interrupts/pic.hpp"

#include "Memory/PagingInit.hpp"

#include <assert.h>

#include "Scheduling/syscall.h"

#include <Scheduling/Scheduler.hpp>

void __attribute__((noreturn)) x86_64_StopRequestHandler() {
    Processor* proc = GetCurrentProcessor();
    proc->StopThis();
}

Processor::Processor(bool BSP) : m_BSP(BSP), m_kernel_stack(nullptr), m_kernel_stack_size(0), m_LocalAPIC(nullptr) {

}

Processor::~Processor() {

}

void Processor::Init(MemoryMapEntry** MemoryMap, uint64_t MMEntryCount, uint64_t kernel_virtual, uint64_t kernel_physical, uint64_t kernel_size, uint64_t HHDM_start, const FrameBuffer& fb) {
    {
        x86_64_GDTInit(&(m_GDT[0]));
        x86_64_GDT_SetTSS(&(m_GDT[0]), &m_TSS);
        x86_64_GDTDescriptor GDTDescriptor;
        GDTDescriptor.Size = sizeof(x86_64_GDTEntry) * 7 - 1;
        GDTDescriptor.Offset = (uint64_t)&(m_GDT[0]);
        x86_64_LoadGDT(&GDTDescriptor);
    }
    if (m_BSP) {
        x86_64_IDT_Initialize();
        x86_64_ISR_Initialize();
        x86_64_IRQ_EarlyInit();
        m_kernel_stack = kernel_stack;
        m_kernel_stack_size = kernel_stack_size;
        x86_64_InitPaging(MemoryMap, MMEntryCount, kernel_virtual, kernel_physical, kernel_size, (uint64_t)(fb.FrameBufferAddress), ((fb.bpp >> 3) * fb.FrameBufferHeight * fb.FrameBufferWidth), HHDM_start);
        x86_64_NMIInit();
    }
    else
        m_kernel_stack_size = KERNEL_STACK_SIZE; // m_kernel_stack is set in the APs early startup
    x86_64_IDT_Load(&idt.idtr);

    m_TSS.RSP[0] = (uint64_t)m_kernel_stack + m_kernel_stack_size;
    x86_64_TSS_Load(0x28);

    assert(x86_64_IsSystemCallSupported());
    assert(x86_64_EnableSystemCalls(0x8, 0x18, x86_64_HandleSystemCall));

    InitialiseLocalAPIC();
}

void Processor::SetLocalAPIC(x86_64_LocalAPIC* LocalAPIC) {
    m_LocalAPIC = LocalAPIC;
}

x86_64_LocalAPIC* Processor::GetLocalAPIC() const {
    return m_LocalAPIC;
}

Processor* GetCurrentProcessor() {
    Scheduling::Scheduler::ProcessorInfo* info = (Scheduling::Scheduler::ProcessorInfo*)x86_64_get_kernel_gs_base();
    return info->processor;
}

Scheduling::Scheduler::ProcessorInfo* GetCurrentProcessorInfo() {
    return (Scheduling::Scheduler::ProcessorInfo*)x86_64_get_kernel_gs_base();
}

uint8_t GetCurrentProcessorID() {
    x86_64_cpuid_regs regs = x86_64_cpuid({0x1, 0, 0, 0});
    return (regs.ebx >> 24) & 0xFF;
}

void Processor::InitialiseLocalAPIC() {
    if (m_LocalAPIC != nullptr) {
        m_LocalAPIC->Init();
        if (!m_BSP)
            Scheduling::Scheduler::AddProcessor(this);
    }
}

void __attribute__((noreturn)) Processor::StopThis() {
    __asm__ volatile("cli");
    while (true)
        __asm__ volatile("hlt");
}
