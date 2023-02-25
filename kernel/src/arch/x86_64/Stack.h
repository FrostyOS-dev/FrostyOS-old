#ifndef _X86_64_KERNEL_STACK_H
#define _X86_64_KERNEL_STACK_H

#ifdef __cplusplus
extern "C" {
#endif

#define KERNEL_STACK_SIZE 65536

void InitKernelStack(unsigned char* kernel_stack_addr, unsigned long stack_size);

extern unsigned char kernel_stack[65536];

#ifdef __cplusplus
}
#endif

#endif /* _X86_64_KERNEL_STACK_H */