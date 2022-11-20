#include "isr.h"
#include <HAL/hal.h>

#include <cstr.h>

#define MODULE "ISR"

x86_64_ISRHandler g_ISRHandlers[256];

static const char* const g_Exceptions[] = {
    "Divide by 0",
    "Reserved",
    "Non-maskable Interrupt",
    "Breakpoint (INT3)",
    "Overflow (INTO)",
    "Bounds range exceeded",
    "Invalid opcode (UD2)",
    "Device not available (WAIT/FWAIT)",
    "Double fault",
    "Coprocessor segment overrun",
    "Invalid TSS",
    "Segment not present",
    "Stack-segment fault",
    "General protection fault",
    "Page fault",
    "Reserved",
    "x87 FPU error",
    "Alignment check",
    "Machine check",
    "SIMD Floating-Point Exception"
};

static const size_t g_ExceptionsSTRLengths[] = {
    12, 9, 23, 18, 16, 22, 21, 34, 13, 28, 12, 20, 20, 25, 11, 9, 14, 16, 14, 30
};

void x86_64_ISR_Initialize() {
    x86_64_ISR_InitializeGates();
    for (int i = 0; i < 256; i++)
        x86_64_IDT_EnableGate(i);
}

void x86_64_ISR_RegisterHandler(uint8_t interrupt, x86_64_ISRHandler handler) {
    g_ISRHandlers[interrupt] = handler;
    x86_64_IDT_EnableGate(interrupt);
}

extern "C" void x86_64_ISR_Handler(x86_64_Registers regs) {
    x86_64_Registers* p_regs = &regs;

    /* Check if there is a designated handler */
    if (g_ISRHandlers[p_regs->interrupt] != nullptr)
        g_ISRHandlers[p_regs->interrupt](p_regs);


    /* TEMP */
    char tempReason[64];
    memset(tempReason, 0, 64);

    const size_t strLength = g_ExceptionsSTRLengths[p_regs->interrupt];

    memcpy(tempReason, g_Exceptions[p_regs->interrupt], strLength);

    WorldOS::Panic(tempReason, p_regs, true);


    while (true); // just hang
}