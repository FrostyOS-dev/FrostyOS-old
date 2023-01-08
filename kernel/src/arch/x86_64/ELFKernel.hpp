#ifndef _KERNEL_X86_64_ELF_KERNEL_HPP
#define _KERNEL_X86_64_ELF_KERNEL_HPP

#include "ELFSymbols.h"

#include <stddef.h>

namespace x86_64_WorldOS {
    bool MapKernel(void* kernel_phys, void* kernel_virt, size_t kernel_size);
}

#endif /* _KERNEL_X86_64_ELF_KERNEL_HPP */