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

#ifndef _X86_64_PROCESSOR_HPP
#define _X86_64_PROCESSOR_HPP

#include <stdint.h>

#include "GDT.hpp"
#include "TSS.hpp"

#include <Memory/Memory.hpp>

#include <Graphics/VGA.hpp>

#include "interrupts/APIC/LocalAPIC.hpp"

namespace Scheduling::Scheduler {
    struct ProcessorInfo;
}

class Processor {
public:
    Processor(bool BSP);
    ~Processor();

    void Init(MemoryMapEntry** MemoryMap, uint64_t MMEntryCount, uint64_t kernel_virtual, uint64_t kernel_physical, uint64_t kernel_size, uint64_t HHDM_start, const FrameBuffer& fb);

    void SetLocalAPIC(x86_64_LocalAPIC* LocalAPIC);

    x86_64_LocalAPIC* GetLocalAPIC() const;

    void InitialiseLocalAPIC();

    static uint64_t get_kernel_stack_offset() {
        return offsetof(Processor, m_kernel_stack);
    }

    void __attribute__((noreturn)) StopThis(); // Stops the current processor

private:
    bool m_BSP;

    void* m_kernel_stack;
    uint64_t m_kernel_stack_size;

    x86_64_TSS m_TSS;

    x86_64_GDTEntry __attribute__((aligned(0x8))) m_GDT[7];

    x86_64_LocalAPIC* m_LocalAPIC;
};

Processor* GetCurrentProcessor();
Scheduling::Scheduler::ProcessorInfo* GetCurrentProcessorInfo();

uint8_t GetCurrentProcessorID();

void __attribute__((noreturn)) x86_64_StopRequestHandler();

#endif /* _X86_64_PROCESSOR_HPP */