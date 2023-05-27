#include <arch/x86_64/GDT/gdt.hpp>

#include <arch/x86_64/interrupts/IDT.hpp>
#include <arch/x86_64/interrupts/isr.hpp>
#include <arch/x86_64/interrupts/IRQ.hpp>
#include <arch/x86_64/interrupts/pic.hpp>

#include <arch/x86_64/fpu.h>
#include <arch/x86_64/io.h>

#include <arch/x86_64/Graphics/vga-graphics.hpp>

#include <arch/x86_64/Memory/PagingInit.hpp>

#include "drivers/ACPI/RSDP.hpp"
#include "drivers/ACPI/XSDT.hpp"

#include "timer.hpp"
#include "hal.hpp"

#include <assert.h>

namespace WorldOS {

    void HAL_EarlyInit(MemoryMapEntry** MemoryMap, uint64_t MMEntryCount, uint64_t kernel_virtual, uint64_t kernel_physical, uint64_t kernel_size, uint64_t HHDM_start, const FrameBuffer& fb) {
        GDT* gdt = &DefaultGDT;
        GDTDescriptor gdtDescriptor = {(sizeof(GDT) - 1), ((uint64_t)gdt)};
        x86_64_LoadGDT(&gdtDescriptor);

        x86_64_IDT_Initialize();
        x86_64_ISR_Initialize();
        x86_64_IDT_Load(&idt.idtr);

        x86_64_FPU_Initialize();

        x86_64_VGA_Graphics_Init(fb, {0,0}, 0xFFFFFFFF, 0);

        x86_64_VGA_Graphics_ClearScreen(0);

        x86_64_IRQ_Initialize();

        x86_64_DisableInterrupts();
        HAL_TimerInit();

        x86_64_InitPaging(MemoryMap, MMEntryCount, kernel_virtual, kernel_physical, kernel_size, (uint64_t)(fb.FrameBufferAddress), ((fb.bpp >> 3) * fb.FrameBufferHeight * fb.FrameBufferWidth), HHDM_start);

        x86_64_EnableInterrupts();
    }

    void HAL_Stage2(void* RSDP) {
        assert(InitAndValidateRSDP(RSDP));
        assert(IsXSDTAvailable());
        assert(InitAndValidateXSDT(GetXSDT()));
    }

    /*
    void Panic(const char* reason, x86_64_Registers* regs, const bool type) {
        x86_64_Panic(reason, regs, type);
    }
    */

}
