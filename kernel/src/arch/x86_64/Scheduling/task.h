/*
Copyright (©) 2022-2023  Frosty515

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

#ifndef _X86_64_TASK_H
#define _X86_64_TASK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct x86_64_Registers {
    uint64_t RAX;
    uint64_t RBX;
    uint64_t RCX;
    uint64_t RDX;
    uint64_t RSI;
    uint64_t RDI;
    uint64_t RSP;
    uint64_t RBP;
    uint64_t R8;
    uint64_t R9;
    uint64_t R10;
    uint64_t R11;
    uint64_t R12;
    uint64_t R13;
    uint64_t R14;
    uint64_t R15;
    uint64_t RIP;
    uint16_t CS;
    uint16_t DS;
    uint64_t RFLAGS;
    uint64_t CR3;
    uint32_t _alignment; // used to fix alignment
} __attribute__((packed));

// Switch to another kernel thread. Does not save any registers and will not return.
void __attribute__((noreturn)) x86_64_kernel_switch(struct x86_64_Registers* regs);

void __attribute__((noreturn)) x86_64_enter_user(struct x86_64_Registers* regs);


// Save all registers. The address to save MUST be placed on the stack before calling.
void x86_64_kernel_save_main();

#define x86_64_kernel_save(r) __asm__ volatile ("push %0" : : "p" (r)); __asm__ volatile ("call x86_64_kernel_save_main")

void* x86_64_get_stack_ptr();

void __attribute__((noreturn)) x86_64_kernel_thread_end();

void x86_64_set_kernel_gs_base(uint64_t base);

#ifdef __cplusplus
}
#endif


#endif /* _X86_64_TASK_H */