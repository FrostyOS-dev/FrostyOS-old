#ifndef _KERNEL_MALLOC_HPP
#define _KERNEL_MALLOC_HPP

#include <stddef.h>

void kmalloc_init();

void* kmalloc(size_t size);
void* kcalloc(size_t size);
void kfree(void* ptr);

#endif /* _KERNEL_MALLOC_HPP */