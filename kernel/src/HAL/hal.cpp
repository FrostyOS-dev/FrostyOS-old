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

#include "hal.hpp"
#include "time.h"

#include "drivers/HPET.hpp"
#include "drivers/PCI.hpp"

#include "drivers/ACPI/ACPI.hpp"
#include "drivers/ACPI/FADT.hpp"
#include "drivers/ACPI/HPET.hpp"
#include "drivers/ACPI/MADT.hpp"
#include "drivers/ACPI/MCFG.hpp"

#include <stdio.h>

#include <arch/x86_64/io.h>
#include <arch/x86_64/panic.hpp>
#include <arch/x86_64/Processor.hpp>

#include <arch/x86_64/Memory/PagingInit.hpp>

#include <arch/x86_64/Scheduling/syscall.h>

#include <Memory/PagingUtil.hpp>

#include <Scheduling/Scheduler.hpp>

#include <tty/TTY.hpp>

#include <uacpi/tables.h>

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
    ACPI_EarlyInit((void*)((uint64_t)RSDP - (uint64_t)g_HHDM_start));
    assert(InitFADT());

    uacpi_table* MADT;
    uacpi_status rc = uacpi_table_find_by_signature("APIC", &MADT);
    if (rc != UACPI_STATUS_OK) {
        printf("MADT table not found: %s\n", uacpi_status_to_string(rc));
        PANIC("MADT table not found");
    }
    assert(InitAndValidateMADT(MADT));
    EnumerateMADTEntries();
    printf("MADT init done\n");

    uacpi_table* HPETTable;
    rc = uacpi_table_find_by_signature("HPET", &HPETTable);
    if (rc != UACPI_STATUS_OK) {
        PANIC("HPET table not found");
    }
    assert(InitAndValidateHPET(HPETTable));

    HPET* hpet = new HPET();
    hpet->Init((HPETRegisters*)GetHPETAddress());
    g_HPET = hpet;

    printf("HPET init done\n");

    uacpi_table* MCFG;
    rc = uacpi_table_find_by_signature("MCFG", &MCFG);
    if (rc != UACPI_STATUS_OK) {
        PANIC("MCFG table not found");
    }
    assert(InitAndValidateMCFG(MCFG));
    
    ACPI_FullInit();

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
        printf("PCI Device: VendorID=%hx DeviceID=%hx Class=%hhx SubClass=%hhx Program Interface=%hhx\n", device->ch.VendorID, device->ch.DeviceID, device->ch.ClassCode, device->ch.SubClass, device->ch.ProgIF);
        device = PCI::PCIDeviceList::GetPCIDevice(i);
    }
}
