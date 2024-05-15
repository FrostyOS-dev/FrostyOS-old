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

#include <arch/x86_64/Processor.hpp>

#include <arch/x86_64/io.h>
#include <arch/x86_64/panic.hpp>

#include <arch/x86_64/Memory/PagingInit.hpp>

#include <arch/x86_64/Scheduling/syscall.h>

#include "drivers/ACPI/ACPI.hpp"
#include "drivers/ACPI/RSDP.hpp"
#include "drivers/ACPI/XSDT.hpp"
#include "drivers/ACPI/MCFG.hpp"
#include "drivers/ACPI/MADT.hpp"
#include "drivers/ACPI/HPET.hpp"

#include "drivers/HPET.hpp"

#include "drivers/PCI.hpp"

#include "time.h"
#include "hal.hpp"

#include <tty/TTY.hpp>

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <Memory/PagingUtil.hpp>

#include <Scheduling/Scheduler.hpp>

Processor g_BSP(true);

void HAL_EarlyInit(MemoryMapEntry** MemoryMap, uint64_t MMEntryCount, uint64_t kernel_virtual, uint64_t kernel_physical, uint64_t kernel_size, uint64_t HHDM_start, const FrameBuffer& fb) {
    x86_64_DisableInterrupts();

    g_BSP = Processor(true);
    g_BSP.Init(MemoryMap, MMEntryCount, kernel_virtual, kernel_physical, kernel_size, HHDM_start, fb);


    x86_64_SetPanicVGADevice(g_CurrentTTY->GetVGADevice());

    x86_64_EnableInterrupts();
}

extern void* g_HHDM_start;

void HAL_Stage2(void* RSDP) {
    assert(InitAndValidateRSDP(RSDP));
    assert(IsXSDTAvailable());
    assert(InitAndValidateXSDT(GetXSDT()));
    bool MCFGFound = false;
    bool MADTFound = false;
    bool HPETFound = false;
    for (uint64_t i = 0; i < getSDTCount(); i++) {
        ACPISDTHeader* header = getOtherSDT(i);
        if (strncmp(header->Signature, "MCFG", 4) == 0 && !MCFGFound) {
            assert(InitAndValidateMCFG(header));
            MCFGFound = true;
        }
        else if (strncmp(header->Signature, "APIC", 4) == 0 && !MADTFound) {
            assert(InitAndValidateMADT(header));
            MADTFound = true;
        }
        else if (strncmp(header->Signature, "HPET", 4) == 0 && !HPETFound) {
            assert(InitAndValidateHPET(header));
            HPETFound = true;
        }
    }
    assert(MCFGFound); // it must be found or device detection won't work
    if (MADTFound)
        EnumerateMADTEntries();
    HPET* hpet = new HPET();
    hpet->Init((HPETRegisters*)GetHPETAddress());
    g_HPET = hpet;
    
    ACPI_init((void*)((uint64_t)RSDP - (uint64_t)g_HHDM_start));

    x86_64_LocalAPIC* LAPIC = g_BSP.GetLocalAPIC();
    HAL_TimeInit();
    LAPIC->InitTimer();

    Scheduling::Scheduler::InitProcessorTimers();
    
}

void HAL_FullInit() {
    for (uint64_t i = 0; i < GetMCFGEntryCount(); i++) {
        MCFGEntry* entry = GetMCFGEntry(i);
        PCI::EnumerateBuses(to_HHDM((void*)(entry->Address)));
    }
    PCI::Header0* device = PCI::PCIDeviceList::GetPCIDevice(0);
    for (uint64_t i = 1; device != nullptr; i++) {
        //dbgprintf("PCI Device: VendorID=%hx DeviceID=%hx Class=%hhx SubClass=%hhx Program Interface=%hhx\n", device->ch.VendorID, device->ch.DeviceID, device->ch.ClassCode, device->ch.SubClass, device->ch.ProgIF);
        device = PCI::PCIDeviceList::GetPCIDevice(i);
    }
}
