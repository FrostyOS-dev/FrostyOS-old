/*
Copyright (©) 2022-2024  Frosty515

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

#include "kernel.hpp"

#include <assert.h>
#include <errno.h>
#include <icxxabi.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <util.h>

#ifdef __x86_64__
#include <arch/x86_64/ELFSymbols.hpp>
#include <arch/x86_64/RTC.hpp>
#endif

#include <fs/FileDescriptorManager.hpp>
#include <fs/initramfs.hpp>
#include <fs/VFS.hpp>

#include <Graphics/Colour.hpp>
#include <Graphics/VGA.hpp>

#include <HAL/time.h>

#include <HAL/drivers/ACPI/ACPI.hpp>

#include <HAL/drivers/PS2/PS2Controller.hpp>

#include <Memory/kmalloc.hpp>
#include <Memory/PageManager.hpp>
#include <Memory/Stack.hpp>

#include <Scheduling/Scheduler.hpp>

#include <SystemCalls/exec.hpp>
#include <SystemCalls/SystemCall.hpp>

#include <tty/KeyboardInput.hpp>
#include <tty/TTY.hpp>

FrameBuffer m_InitialFrameBuffer;
Colour g_fgcolour;
Colour g_bgcolour;
uint64_t m_Stage;
ColourFormat g_ColourFormat;

PageManager KPM;

Scheduling::Process* KProcess;

BasicVGA KBasicVGA;

TTY KTTY;

FileDescriptorManager KFDManager;
uint8_t KFDManager_BitmapData[8]; // allows up to 64 file descriptors
FileDescriptor Kstdin;
FileDescriptor Kstdout;
FileDescriptor Kstderr;
FileDescriptor Kstddebug;

PS2Controller* KPS2Controller;

KeyboardInput KInput;

VFS_WorkingDirectory* KWorkingDirectory;

struct Stage2_Params {
    void* RSDP_addr;
    void* initramfs_addr;
    size_t initramfs_size;
} Kernel_Stage2Params;

extern "C" void StartKernel(KernelParams* params) {
    {
        typedef void (*ctor_fn)();
        ctor_fn* ctors = (ctor_fn*)_ctors_start_addr;
        uint64_t ctors_count = ((uint64_t)_ctors_end_addr - (uint64_t)_ctors_start_addr) / sizeof(ctor_fn);
        for (uint64_t i = 0; i < ctors_count; i++)
            ctors[i]();
    }

    m_Stage = EARLY_STAGE;
    m_InitialFrameBuffer = params->frameBuffer;
    g_ColourFormat = ColourFormat(m_InitialFrameBuffer.bpp, m_InitialFrameBuffer.red_mask_shift, m_InitialFrameBuffer.red_mask_size, m_InitialFrameBuffer.green_mask_shift, m_InitialFrameBuffer.green_mask_size, m_InitialFrameBuffer.blue_mask_shift, m_InitialFrameBuffer.blue_mask_size);
    g_fgcolour = Colour(g_ColourFormat, 0xFF, 0xFF, 0xFF);
    g_bgcolour = Colour(g_ColourFormat, 0, 0, 0);

    KBasicVGA.Init(m_InitialFrameBuffer, {0, 0}, g_fgcolour, g_bgcolour);

    KTTY = TTY(&KBasicVGA, nullptr, g_fgcolour, g_bgcolour); 

    g_CurrentTTY = &KTTY;

    LinkedList::NodePool_Init();

    KFDManager = FileDescriptorManager(KFDManager_BitmapData, 8);
    Kstdin = FileDescriptor(FileDescriptorType::TTY, &KTTY, FileDescriptorMode::READ, 0);
    Kstdout = FileDescriptor(FileDescriptorType::TTY, &KTTY, FileDescriptorMode::WRITE, 1);
    Kstderr = FileDescriptor(FileDescriptorType::TTY, &KTTY, FileDescriptorMode::WRITE, 2);
    Kstddebug = FileDescriptor(FileDescriptorType::DEBUG, nullptr, FileDescriptorMode::WRITE, 3);
    assert(KFDManager.ReserveFileDescriptor(&Kstdin));
    assert(KFDManager.ReserveFileDescriptor(&Kstdout));
    assert(KFDManager.ReserveFileDescriptor(&Kstderr));
    assert(KFDManager.ReserveFileDescriptor(&Kstddebug));

    g_KFDManager = &KFDManager;

    uint64_t kernel_size = (uint64_t)_kernel_end_addr - (uint64_t)_text_start_addr;

    g_KPT = PageTable(false, nullptr);
    Scheduling::Scheduler::ClearGlobalData();
    Scheduling::Scheduler::InitBSPInfo();
    params->MemoryMapEntryCount = UpdateMemorySize(const_cast<const MemoryMapEntry**>(params->MemoryMap), params->MemoryMapEntryCount);
    HAL_EarlyInit(params->MemoryMap, params->MemoryMapEntryCount, params->kernel_virtual_addr, params->kernel_physical_addr, kernel_size, params->hhdm_start_addr, m_InitialFrameBuffer);
    KPM.InitPageManager(VirtualRegion((void*)(params->kernel_virtual_addr + kernel_size), (void*)UINT64_MAX), g_KVPM, false);
    g_KPM = &KPM;
    kmalloc_eternal_init();
    kmalloc_init();

    if (params->frameBuffer.bpp % 8 > 0 || params->frameBuffer.bpp > 64) {
        PANIC("Bootloader Frame Buffer Bits per Pixel is either not byte aligned or larger than 64");
    }

    // Do any early initialisation

    g_BSP.UpdateKernelStack(CreateKernelStack());
    g_BSP.EnableExceptionProtection();

    printf("Very early init done.\n");

    KBasicVGA.EnableDoubleBuffering(g_KPM);

    HAL_Stage2(params->RSDP_table);

    KPS2Controller = new PS2Controller();
    KPS2Controller->Init();
    printf("Detected ");
    KPS2Controller->PrintDeviceInfo(stdout);

    KInput = KeyboardInput();
    KInput.Initialise((Keyboard*)KPS2Controller->GetKeyboard());

    KTTY.SetKeyboardInput(&KInput);

    uint8_t* ELF_map_data;
    uint64_t ELF_map_size = TarFS::USTAR_Lookup((uint8_t*)(params->initramfs_addr), "kernel.map", &ELF_map_data);
    if (ELF_map_size == 0)
        dbgprintf("WARN: Cannot find kernel symbol file\n");
    else
        g_KernelSymbols = new ELFSymbols(ELF_map_data, ELF_map_size, true);

    KWorkingDirectory = nullptr;

    Kernel_Stage2Params = {
        .RSDP_addr = params->RSDP_table,
        .initramfs_addr = params->initramfs_addr,
        .initramfs_size = params->initramfs_size
    };

    KProcess = new Scheduling::Process(Kernel_Stage2, (void*)&Kernel_Stage2Params, 0, 0, Scheduling::Priority::KERNEL, Scheduling::KERNEL_DEFAULT, g_KPM);
    KProcess->SetDefaultWorkingDirectory(KWorkingDirectory);
    KProcess->Start();

    for (uint8_t i = 0; i < (Scheduling::Scheduler::GetProcessorCount() - 1); i++) {
        Scheduling::Thread* thread = new Scheduling::Thread(KProcess, nullptr, nullptr, Scheduling::THREAD_KERNEL_DEFAULT);
        Scheduling::Scheduler::AddIdleThread(thread);
    }
    
    Scheduling::Scheduler::Start();

    PANIC("Scheduler Start returned!\n");
}

void Kernel_Stage2(void* params_addr) {
    //dbgputs("Starting WorldOS!\n");
    puts("Starting WorldOS!\n");
    printf("Build time: %s %s\n", __DATE__, __TIME__);
    //dbgprintf("Build time: %s %s\n", __DATE__, __TIME__);

    m_Stage = STAGE2;

    Stage2_Params* params = (Stage2_Params*)params_addr;

    HAL_FullInit();

    VFS* KVFS = (VFS*)kcalloc_eternal(1, sizeof(VFS));
    g_VFS = KVFS;
    assert(KVFS->MountRoot(FileSystemType::TMPFS) == ESUCCESS);

    puts("VFS root mounted.\n");

    KWorkingDirectory = KVFS->GetRootWorkingDirectory();
    KProcess->SetDefaultWorkingDirectory(KWorkingDirectory);

    Initialise_InitRAMFS(params->initramfs_addr, params->initramfs_size);

    puts("Initial RAMFS initialised.\n");

    SystemCallInit();

    puts("Sleeping for 10 seconds...\n");

    RTCTime t = RTC_getCurrentTime();
    time_t time = ((days_since_epoch(t.Year, t.Month, t.DayOfMonth) * 24 + t.Hours) * 60 + t.Minutes) * 60 + t.Seconds;
    while ((time + 10) > (((days_since_epoch(t.Year, t.Month, t.DayOfMonth) * 24 + t.Hours) * 60 + t.Minutes) * 60 + t.Seconds)) {
        t = RTC_getCurrentTime();
    }


    puts("Attempting ACPI Shutdown!\n");

    int rc = Shutdown();
    if (rc != 0) {
        printf("ACPI Shutdown failed with error code %s (%d)\n", strerror(-rc), rc);
    }

    while (true) {
        
    }
}
