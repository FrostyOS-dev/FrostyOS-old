#ifndef _X86_64_CPUID_HPP
#define _X86_64_CPUID_HPP

#include <stdint.h>

struct x86_64_cpuid_regs {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
} __attribute__((packed));

// Defined in cpuid.asm

// Call cpuid with given registers
extern "C" x86_64_cpuid_regs x86_64_cpuid(x86_64_cpuid_regs regs);

#endif /* _X86_64_CPUID_HPP */