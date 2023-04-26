#include "Stack.h"

unsigned char __attribute__((aligned(0x1000))) kernel_stack[KERNEL_STACK_SIZE] = {0};
unsigned long int kernel_stack_size = KERNEL_STACK_SIZE;
