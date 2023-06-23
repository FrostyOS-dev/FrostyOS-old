#include "taskutil.hpp"

#include "../interrupts/isr.hpp"

#include <util.h>
#include <assert.h>

void x86_64_PrepareNewRegisters(x86_64_Interrupt_Registers* out, const x86_64_Registers* in) {
    out->RAX = in->RAX;
    out->RBX = in->RBX;
    out->RCX = in->RCX;
    out->RDX = in->RDX;
    out->RSI = in->RSI;
    out->RDI = in->RDI;
    out->rsp = in->RSP;
    out->RBP = in->RBP;
    out->R8 = in->R8;
    out->R9 = in->R9;
    out->R10 = in->R10;
    out->R11 = in->R11;
    out->R12 = in->R12;
    out->R13 = in->R13;
    out->R14 = in->R14;
    out->R15 = in->R15;
    out->rip = in->RIP;
    out->rflags = in->RFLAGS;
    out->CR3 = in->CR3;
    out->cs = in->CS;
    out->ds = in->DS;
}

void x86_64_ConvertToStandardRegisters(x86_64_Registers* out, const x86_64_Interrupt_Registers* in) {
    out->RAX = in->RAX;
    out->RBX = in->RBX;
    out->RCX = in->RCX;
    out->RDX = in->RDX;
    out->RSI = in->RSI;
    out->RDI = in->RDI;
    out->RSP = in->rsp;
    out->RBP = in->RBP;
    out->R8 = in->R8;
    out->R9 = in->R9;
    out->R10 = in->R10;
    out->R11 = in->R11;
    out->R12 = in->R12;
    out->R13 = in->R13;
    out->R14 = in->R14;
    out->R15 = in->R15;
    out->RIP = in->rip;
    out->RFLAGS = in->rflags;
    out->CR3 = in->CR3;
    out->CS = in->cs;
    out->DS = in->ds;
}

void x86_64_GetNewStack(WorldOS::PageManager* pm, x86_64_Registers* regs, size_t size) {
    size = ALIGN_UP(size, 4096);
    if (size < KiB(16))
        size = KiB(16);
    void* stack = pm->AllocatePages(size / 4096);
    assert(stack != nullptr);
    regs->RSP = (uint64_t)stack + size; // stack grows downwards on x86
}
