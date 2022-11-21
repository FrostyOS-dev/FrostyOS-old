#include "hal.hpp"

#include <arch/x86_64/GDT/gdt.hpp>
#include <arch/x86_64/interrupts/IDT.hpp>
#include <arch/x86_64/interrupts/isr.hpp>
#include <arch/x86_64/interrupts/IRQ.hpp>
#include <arch/x86_64/interrupts/pic.hpp>
#include <arch/x86_64/fpu.h>

#include <arch/x86_64/Graphics/vga-graphics.hpp>

namespace WorldOS {

    void HAL_Init(const FrameBuffer& fb) {
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
    }

    /*
    void Panic(const char* reason, x86_64_Registers* regs, const bool type) {
        x86_64_Panic(reason, regs, type);
    }
    */

}