#include "hal.h"

#include <arch/x86_64/GDT/gdt.h>
#include <arch/x86_64/IDT/IDT.h>
#include <arch/x86_64/IDT/isr.h>
#include <arch/x86_64/fpu.h>

namespace WorldOS {
    void HAL_Init() {
        GDT* gdt = &DefaultGDT;
        GDTDescriptor gdtDescriptor = {(sizeof(GDT) - 1), ((uint64_t)gdt)};
        x86_64_LoadGDT(&gdtDescriptor);

        x86_64_IDT_Initialize();
        x86_64_ISR_Initialize();
        x86_64_IDT_Load(&idt.idtr);

        x86_64_FPU_Initialize();
    }

    /*
    void Panic(const char* reason, x86_64_Registers* regs, const bool type) {
        x86_64_Panic(reason, regs, type);
    }
    */
}