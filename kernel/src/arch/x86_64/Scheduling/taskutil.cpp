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

#include "taskutil.hpp"

#include "../interrupts/isr.hpp"
#include "Scheduling/Semaphore.hpp"

#include <util.h>
#include <assert.h>
#include <stdlib.h>

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
    out->ss = in->DS;
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

void x86_64_GetNewStack(PageManager* pm, x86_64_Registers* regs, size_t size) {
    size = ALIGN_UP(size, 4096);
    if (size < KiB(16))
        size = KiB(16);
    void* stack_range_start = pm->ReservePages(size / PAGE_SIZE + 2, PagePermissions::READ_WRITE);
    void* stack = pm->AllocatePages(size / PAGE_SIZE, PagePermissions::READ_WRITE, (void*)((uint64_t)stack_range_start + PAGE_SIZE));
    assert(stack != nullptr);
    regs->RSP = (uint64_t)stack + size; // stack grows downwards on x86
}

void x86_64_SaveIRegistersToThread(const Scheduling::Thread* thread, const x86_64_Interrupt_Registers* in) {
    x86_64_Registers* out = thread->GetCPURegisters();
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

void* x86_64_GetSignalReturnInstructions(size_t* size, int signum) {
    /* 
    These are the instructions that get generated:
    ```
    mov rax, SC_SIGRETURN(29) (MOV r/m64, imm32) (REX.W + C7 /0 id)
    mov edi, signum (MOV r32, imm32) (B8 + rd id)
    syscall
    ```
    We need to allocate a buffer big enough to hold these instructions.
    */
    uint8_t* buf = (uint8_t*)kmalloc(14);
    assert(buf != nullptr);
    buf[0] = 0x48;
    buf[1] = 0xC7;
    buf[2] = 0xC0;
    buf[3] = 0x1D;
    buf[4] = 0x00;
    buf[5] = 0x00;
    buf[6] = 0x00;
    buf[7] = 0xBF;
    buf[8] = signum & 0xFF;
    buf[9] = (signum >> 8) & 0xFF;
    buf[10] = (signum >> 16) & 0xFF;
    buf[11] = (signum >> 24) & 0xFF;
    buf[12] = 0x0F;
    buf[13] = 0x05;
    *size = 14;
    return buf;
}

extern "C" void x86_64_HandleSemaphoreAcquire(Scheduling::Semaphore* semaphore, Scheduling::Thread* thread, x86_64_Registers* regs) {
    if (regs != nullptr) {
        memcpy(thread->GetCPURegisters(), regs, sizeof(x86_64_Registers));
        semaphore->acquire(thread);
    }
}