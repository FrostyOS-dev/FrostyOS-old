#include "kernel.hpp"

#include <arch/x86_64/ELFSymbols.h>

#include <Memory/PageManager.hpp>
#include <Memory/kmalloc.hpp>

#include <HAL/drivers/ACPI/RSDP.hpp>
#include <HAL/drivers/ACPI/XSDT.hpp>

#include <HAL/time.h>

#include <Scheduling/Scheduler.hpp>

#include <Graphics/VGA.hpp>
#include <Graphics/Colour.hpp>

#include <tty/TTY.hpp>

#include <assert.h>

namespace WorldOS {

    FrameBuffer m_InitialFrameBuffer;
    Colour g_fgcolour;
    Colour g_bgcolour;
    uint64_t m_Stage;

    PageManager KPM;

    Scheduling::Process* KProcess;

    BasicVGA KBasicVGA;

    TTY KTTY;

    extern "C" void StartKernel(KernelParams* params) {
        g_fgcolour = Colour(0xFF, 0xFF, 0xFF);
        g_bgcolour = Colour(0, 0, 0);
        m_Stage = EARLY_STAGE;
        m_InitialFrameBuffer = params->frameBuffer;

        KBasicVGA.Init(m_InitialFrameBuffer, {0, 0}, g_fgcolour.as_ARGB(), g_bgcolour.as_ARGB());

        KTTY = TTY(&KBasicVGA, g_fgcolour, g_bgcolour); 

        g_CurrentTTY = &KTTY;

        uint64_t kernel_size = (uint64_t)_kernel_end_addr - (uint64_t)_text_start_addr;

        HAL_EarlyInit(params->MemoryMap, params->MemoryMapEntryCount, params->kernel_virtual_addr, params->kernel_physical_addr, kernel_size, params->hhdm_start_addr, m_InitialFrameBuffer);
        KPM.InitPageManager((void*)(params->kernel_virtual_addr + kernel_size), UINT64_MAX - (params->kernel_virtual_addr + kernel_size), false);
        g_KPM = &KPM;
        kmalloc_init();

        if (params->frameBuffer.bpp != 32) {
            Panic("Bootloader Frame Buffer Bits per Pixel is not 32", nullptr, false);
        }

        // Do any early initialisation

        KBasicVGA.EnableDoubleBuffering(g_KPM);

        KProcess = new Scheduling::Process(Kernel_Stage2, params->RSDP_table, Scheduling::Priority::KERNEL, Scheduling::KERNEL_DEFAULT, g_KPM);
        KProcess->Start();

        Scheduling::Scheduler::Start();

        Panic("Scheduler Start returned!\n", nullptr, false);
    }

    void Kernel_Stage2(void* RSDP_table) {
        fputs(VFS_DEBUG_AND_STDOUT, "Starting WorldOS!\n");

        m_Stage = STAGE2;

        HAL_Stage2(RSDP_table);

        Panic("Test", nullptr, false);

        while (true) {
            
        }
    }
}
