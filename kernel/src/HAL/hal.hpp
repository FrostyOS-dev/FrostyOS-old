/*
Copyright (©) 2022-2023  Frosty515

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

#ifndef _KERNEL_HAL_HPP
#define _KERNEL_HAL_HPP

#include <util.h>

#ifdef _FROSTYOS_BUILD_TARGET_IS_KERNEL
#ifdef __x86_64__
#include <arch/x86_64/panic.hpp>
#include <arch/x86_64/Scheduling/task.h>
#include <arch/x86_64/Processor.hpp>

typedef x86_64_Registers CPU_Registers;
#endif

#include <stdint.h>
#include <Memory/Memory.hpp>

#include <Graphics/Graphics.h>


// Get the essentials running. This is (in order): GDT, IDT, ISR, IRQ, PIT Timer, RTC clock, PMM, Kernel Paging maps, KVPM.
void HAL_EarlyInit(MemoryMapEntry** MemoryMap, uint64_t MMEntryCount, uint64_t kernel_virtual, uint64_t kernel_physical, uint64_t kernel_size, uint64_t HHDM_start, const FrameBuffer& fb);

// Initialise essential features that need to be enabled after the heap. This is (in order): RSDP, XSDT, SMP. MUST BE CALLED AFTER KPM AND HEAP ARE READY.
void HAL_Stage2(void* RSDP);

// Initials less-essential features. This is (in order): PCI/PCIe
void HAL_FullInit();

void PrepareForShutdown();

int Shutdown();

extern Processor g_BSP;
#endif

#ifdef _FROSTYOS_BUILD_TARGET_IS_USERLAND

#include <HAL/UserPanic.hpp>



void __attribute__((noreturn)) Shutdown();

#endif

#endif /* _KERNEL_HAL_HPP */