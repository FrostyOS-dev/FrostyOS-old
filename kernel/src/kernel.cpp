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
#include "Memory/VirtualPageManager.hpp"
#include "fs/FileDescriptor.hpp"

#include <assert.h>
#include <icxxabi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <util.h>
#include <errno.h>

#ifdef _FROSTYOS_BUILD_TARGET_IS_KERNEL
#ifdef __x86_64__
#include <arch/x86_64/RTC.hpp>
#endif

#include <Graphics/Colour.hpp>
#include <Graphics/VGA.hpp>

#include <HAL/time.h>

#include <HAL/drivers/ACPI/ACPI.hpp>

#include <HAL/drivers/PS2/PS2Controller.hpp>

#include <Memory/Stack.hpp>

#include <Scheduling/Scheduler.hpp>

#include <SystemCalls/exec.hpp>
#include <SystemCalls/SystemCall.hpp>

#include <tty/backends/KeyboardInput.hpp>
#include <tty/backends/BochsDebugTTY.hpp>
#include <tty/backends/VGATTY.hpp>
#elif defined(_FROSTYOS_BUILD_TARGET_IS_USERLAND)
#include <tty/backends/UserTTY.hpp>
#endif

#ifdef __x86_64__
#include <arch/x86_64/ELFSymbols.hpp>
#endif

#include <tty/TTY.hpp>

#include <Memory/kmalloc.hpp>
#include <Memory/PageManager.hpp>
#include <fs/FileDescriptorManager.hpp>
#include <fs/initramfs.hpp>
#include <fs/VFS.hpp>

#ifdef _FROSTYOS_BUILD_TARGET_IS_KERNEL
FrameBuffer m_InitialFrameBuffer;
Colour g_fgcolour;
Colour g_bgcolour;
ColourFormat g_ColourFormat;


Scheduling::Process* KProcess;

BasicVGA KBasicVGA;
TTYBackendVGA KTTYBackendVGA;
TTYBackendBochsDebug KTTYBackendBochsDebug;

PS2Controller* KPS2Controller;

KeyboardInput KInput;
#elif defined(_FROSTYOS_BUILD_TARGET_IS_USERLAND)
UserTTY KUserTTYInput;
UserTTY KUserTTYOutput;
UserTTY KUserTTYDebug;

VirtualPageManager* vpm;
#endif

PageManager KPM;

TTY KTTY;

uint64_t m_Stage;

TTYFileDescriptor KINTTYFileDescriptor;
TTYFileDescriptor KOUTTTYFileDescriptor;
TTYFileDescriptor KERRTTYFileDescriptor;
TTYFileDescriptor KDEBUGTTYFileDescriptor;

FileDescriptorManager KFDManager;
uint8_t KFDManager_BitmapData[8]; // allows up to 64 file descriptors
FileDescriptor Kstdin;
FileDescriptor Kstdout;
FileDescriptor Kstderr;
FileDescriptor Kstddebug;

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
#ifdef _FROSTYOS_BUILD_TARGET_IS_KERNEL
    m_InitialFrameBuffer = params->frameBuffer;
    g_ColourFormat = ColourFormat(m_InitialFrameBuffer.bpp, m_InitialFrameBuffer.red_mask_shift, m_InitialFrameBuffer.red_mask_size, m_InitialFrameBuffer.green_mask_shift, m_InitialFrameBuffer.green_mask_size, m_InitialFrameBuffer.blue_mask_shift, m_InitialFrameBuffer.blue_mask_size);
    g_fgcolour = Colour(g_ColourFormat, 0xFF, 0xFF, 0xFF);
    g_bgcolour = Colour(g_ColourFormat, 0, 0, 0);

    KBasicVGA.Init(m_InitialFrameBuffer, {0, 0}, g_fgcolour, g_bgcolour);
    KTTYBackendVGA.SetVGADevice(&KBasicVGA);
    KTTYBackendVGA.SetDefaultForeground(g_fgcolour);
    KTTYBackendVGA.SetDefaultBackground(g_bgcolour);

    KTTY.SetBackend(nullptr, TTYBackendMode::IN);
    KTTY.SetBackend(&KTTYBackendVGA, TTYBackendMode::OUT);
    KTTY.SetBackend(&KTTYBackendVGA, TTYBackendMode::ERR);
    KTTY.SetBackend(&KTTYBackendBochsDebug, TTYBackendMode::DEBUG);
#elif defined(_FROSTYOS_BUILD_TARGET_IS_USERLAND)
    KUserTTYInput = UserTTY(TTYBackendStreamDirection::INPUT, 0);
    KUserTTYOutput = UserTTY(TTYBackendStreamDirection::OUTPUT, 1);
    KUserTTYDebug = UserTTY(TTYBackendStreamDirection::OUTPUT, 2);

    KTTY.SetBackend(&KUserTTYInput, TTYBackendMode::IN);
    KTTY.SetBackend(&KUserTTYOutput, TTYBackendMode::OUT);
    KTTY.SetBackend(&KUserTTYOutput, TTYBackendMode::ERR);
    KTTY.SetBackend(&KUserTTYDebug, TTYBackendMode::DEBUG);
#endif

    KINTTYFileDescriptor = {
        .tty = &KTTY,
        .mode = TTYBackendMode::IN
    };
    KOUTTTYFileDescriptor = {
        .tty = &KTTY,
        .mode = TTYBackendMode::OUT
    };
    KERRTTYFileDescriptor = {
        .tty = &KTTY,
        .mode = TTYBackendMode::ERR
    };
    KDEBUGTTYFileDescriptor = {
        .tty = &KTTY,
        .mode = TTYBackendMode::DEBUG
    };

    g_CurrentTTY = &KTTY;

    LinkedList::NodePool_Init();

    KFDManager = FileDescriptorManager(KFDManager_BitmapData, 8);

    Kstdin = FileDescriptor(FileDescriptorType::TTY, &KINTTYFileDescriptor, FileDescriptorMode::READ, 0);
    Kstdout = FileDescriptor(FileDescriptorType::TTY, &KOUTTTYFileDescriptor, FileDescriptorMode::WRITE, 1);
    Kstderr = FileDescriptor(FileDescriptorType::TTY, &KERRTTYFileDescriptor, FileDescriptorMode::WRITE, 2);
    Kstddebug = FileDescriptor(FileDescriptorType::TTY, &KDEBUGTTYFileDescriptor, FileDescriptorMode::WRITE, 3);
    assert(KFDManager.ReserveFileDescriptor(&Kstdin));
    assert(KFDManager.ReserveFileDescriptor(&Kstdout));
    assert(KFDManager.ReserveFileDescriptor(&Kstderr));
    assert(KFDManager.ReserveFileDescriptor(&Kstddebug));

    g_KFDManager = &KFDManager;

#ifdef _FROSTYOS_BUILD_TARGET_IS_KERNEL
    uint64_t kernel_size = (uint64_t)_kernel_end_addr - (uint64_t)_text_start_addr;

    g_KPT = PageTable(false, nullptr);
    Scheduling::Scheduler::ClearGlobalData();
    Scheduling::Scheduler::InitBSPInfo();
    params->MemoryMapEntryCount = UpdateMemorySize(const_cast<const MemoryMapEntry**>(params->MemoryMap), params->MemoryMapEntryCount);
    HAL_EarlyInit(params->MemoryMap, params->MemoryMapEntryCount, params->kernel_virtual_addr, params->kernel_physical_addr, kernel_size, params->hhdm_start_addr, m_InitialFrameBuffer);
    KPM.InitPageManager(VirtualRegion((void*)(params->kernel_virtual_addr + kernel_size), (void*)UINT64_MAX), g_KVPM, false);
#elif defined(_FROSTYOS_BUILD_TARGET_IS_USERLAND)
    kmalloc_vmm_init();
    KPM.InitPageManager();
#endif
    g_KPM = &KPM;
    kmalloc_eternal_init();
    kmalloc_init();

#ifdef _FROSTYOS_BUILD_TARGET_IS_KERNEL
    if (params->frameBuffer.bpp % 8 > 0 || params->frameBuffer.bpp > 64) {
        PANIC("Bootloader Frame Buffer Bits per Pixel is either not byte aligned or larger than 64");
    }

    // Do any early initialisation

    g_BSP.UpdateKernelStack(CreateKernelStack());
    g_BSP.EnableExceptionProtection();
    
    KBasicVGA.EnableDoubleBuffering(g_KPM);


    HAL_Stage2(params->RSDP_table);

    KPS2Controller = new PS2Controller();
    KPS2Controller->Init();
    printf("Detected ");
    KPS2Controller->PrintDeviceInfo(stdout);

    KInput = KeyboardInput();
    KInput.Initialise((Keyboard*)KPS2Controller->GetKeyboard());

    KTTY.SetBackend(&KInput, TTYBackendMode::IN);
#endif

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

#ifdef _FROSTYOS_BUILD_TARGET_IS_KERNEL
    KProcess = new Scheduling::Process(Kernel_Stage2, (void*)&Kernel_Stage2Params, 0, 0, Scheduling::Priority::KERNEL, Scheduling::KERNEL_DEFAULT, g_KPM);
    KProcess->SetDefaultWorkingDirectory(KWorkingDirectory);
    KProcess->Start();

    for (uint8_t i = 0; i < (Scheduling::Scheduler::GetProcessorCount() - 1); i++) {
        Scheduling::Thread* thread = new Scheduling::Thread(KProcess, nullptr, nullptr, Scheduling::THREAD_KERNEL_DEFAULT);
        Scheduling::Scheduler::AddIdleThread(thread);
    }
    
    Scheduling::Scheduler::Start();

    PANIC("Scheduler Start returned!\n");
#else
    Kernel_Stage2((void*)&Kernel_Stage2Params);
#endif
}

void Kernel_Stage2(void* params_addr) {
    dbgputs("Starting FrostyOS!\n");
    puts("Starting FrostyOS!\n");

    m_Stage = STAGE2;

    Stage2_Params* params = (Stage2_Params*)params_addr;

#ifdef _FROSTYOS_BUILD_TARGET_IS_KERNEL
    HAL_FullInit();
#endif

    VFS* KVFS = (VFS*)kcalloc_eternal(1, sizeof(VFS));
    g_VFS = KVFS;
    assert(KVFS->MountRoot(FileSystemType::TMPFS) == ESUCCESS);

    puts("VFS root mounted.\n");

    KWorkingDirectory = KVFS->GetRootWorkingDirectory();
#ifdef _FROSTYOS_BUILD_TARGET_IS_KERNEL
    KProcess->SetDefaultWorkingDirectory(KWorkingDirectory);
#endif

    Initialise_InitRAMFS(params->initramfs_addr, params->initramfs_size);

    puts("Initial RAMFS initialised.\n");

#ifdef _FROSTYOS_BUILD_TARGET_IS_KERNEL
    SystemCallInit();
#endif

    while (true) {
        
    }
}
