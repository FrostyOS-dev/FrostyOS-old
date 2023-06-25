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
} __attribute__((packed));

// Switch to another kernel thread. Does not save any registers and will not return.
void __attribute__((noreturn)) x86_64_kernel_switch(struct x86_64_Registers* regs);


// Save all registers. The address to save MUST be placed on the stack before calling.
void x86_64_kernel_save_main();

#define x86_64_kernel_save(r) __asm__ volatile ("push %0" : : "p" (r)); __asm__ volatile ("call x86_64_kernel_save_main")

void* x86_64_get_stack_ptr();

void __attribute__((noreturn)) x86_64_kernel_thread_end();

#ifdef __cplusplus
}
#endif


#endif /* _X86_64_TASK_H */