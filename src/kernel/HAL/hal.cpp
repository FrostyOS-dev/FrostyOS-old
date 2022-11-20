#include "hal.h"

#include <arch/x86_64/GDT/gdt.h>
#include <arch/x86_64/interrupts/IDT.h>
#include <arch/x86_64/interrupts/isr.h>
#include <arch/x86_64/interrupts/IRQ.h>
#include <arch/x86_64/interrupts/pic.h>
#include <arch/x86_64/fpu.h>
#include <arch/x86_64/E9.h>

#include <arch/x86_64/Graphics/vga-graphics.h>

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

    void debug_putc(const char c) {
        x86_64_debug_putc(c);
    }

    void debug_puts(const char* str) {
        x86_64_debug_puts(str);
    }

}