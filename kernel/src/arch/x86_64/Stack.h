#ifndef _X86_64_KERNEL_STACK_H
#define _X86_64_KERNEL_STACK_H

#ifdef __cplusplus
extern "C" {
#endif

#define KERNEL_STACK_SIZE 65536

extern unsigned char __attribute__((aligned(0x1000))) kernel_stack[KERNEL_STACK_SIZE];

extern unsigned long int kernel_stack_size;

#ifdef __cplusplus
}
#endif

#endif /* _X86_64_KERNEL_STACK_H */