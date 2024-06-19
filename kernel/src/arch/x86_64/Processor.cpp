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

#include "Memory/PagingInit.hpp"
#include "Memory/PAT.hpp"

#include <assert.h>
#include <util.h>

#include "Scheduling/syscall.h"

#include <Memory/Stack.hpp>

#include <Scheduling/Scheduler.hpp>

Processor::Processor(bool BSP) : m_BSP(BSP), m_kernel_stack(nullptr), m_kernel_stack_size(0), m_LocalAPIC(nullptr), m_IPIList() {

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
    x86_64_InitPAT();
    if (m_BSP) {
        x86_64_IDT_Initialize();
        x86_64_ISR_Initialize();
        x86_64_IRQ_EarlyInit();
        m_kernel_stack = kernel_stack;
        m_kernel_stack_size = kernel_stack_size;
        x86_64_InitPaging(MemoryMap, MMEntryCount, kernel_virtual, kernel_physical, kernel_size, (uint64_t)(fb.FrameBufferAddress), ALIGN_UP((fb.FrameBufferHeight * fb.pitch), PAGE_SIZE), HHDM_start);
        x86_64_NMIInit();
    }
    else {
        m_kernel_stack_size = KERNEL_STACK_SIZE; // m_kernel_stack is set in the APs early startup
        EnableExceptionProtection();
    }
    x86_64_IDT_Load(&idt.idtr);

    m_TSS.RSP[0] = (uint64_t)m_kernel_stack + m_kernel_stack_size;
    x86_64_TSS_Load(0x28);

    assert(x86_64_IsSystemCallSupported());
    assert(x86_64_EnableSystemCalls(0x8, 0x18, x86_64_HandleSystemCall));

    m_IPIList = x86_64_IPI_List();

    if (!m_BSP)
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
    assert(m_LocalAPIC != nullptr);
    m_LocalAPIC->Init();
    if (!m_BSP) {
        Scheduling::Scheduler::AddProcessor(this);
        PANIC("Scheduler::AddProcessor failed. This should not happen under any circumstance. Something is very wrong.");
    }
}

void __attribute__((noreturn)) Processor::StopThis() {
    __asm__ volatile("cli");
    while (true)
        __asm__ volatile("hlt");
}

x86_64_IPI_List& Processor::GetIPIList() {
    return m_IPIList;
}

void Processor::UpdateKernelStack(void* stack, uint64_t stack_size) {
    m_kernel_stack = stack;
    m_kernel_stack_size = stack_size;
    m_TSS.RSP[0] = (uint64_t)m_kernel_stack + m_kernel_stack_size;
}

void Processor::EnableExceptionProtection() {
    // Use alternative ISTs for Page Faults, General Protection Faults and Double Faults
    // First create the alternative stacks and set them in the TSS
    m_double_fault_stack = CreateKernelStack();
    m_general_protection_fault_stack = CreateKernelStack();
    m_page_fault_stack = CreateKernelStack();
    m_TSS.IST[0] = (uint64_t)m_double_fault_stack + KERNEL_STACK_SIZE;
    m_TSS.IST[1] = (uint64_t)m_general_protection_fault_stack + KERNEL_STACK_SIZE;
    m_TSS.IST[2] = (uint64_t)m_page_fault_stack + KERNEL_STACK_SIZE;

    if (m_BSP) { // only need to be set for the BSP
        idt.entries[0x8].ist = 1;
        idt.entries[0xD].ist = 2;
        idt.entries[0xE].ist = 3;
    }
}

extern "C" void* GetRealKernelStack(Processor* processor) {
    return (void*)((uint64_t)processor->GetKernelStack() + KERNEL_STACK_SIZE);
}
