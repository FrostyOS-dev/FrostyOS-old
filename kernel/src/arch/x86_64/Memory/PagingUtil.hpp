#ifndef _KERNEL_X86_64_PAGING_UTIL_HPP
#define _KERNEL_X86_64_PAGING_UTIL_HPP

#include <stdint.h>

// Defined in C++ Source file

void x86_64_GeneratePageLevel3Array(uint16_t PageLevel4Offset);
void x86_64_GeneratePageLevel2Array(uint16_t PageLevel4Offset, uint16_t PageLevel3Offset);
void x86_64_GeneratePageLevel1Array(uint16_t PageLevel4Offset, uint16_t PageLevel3Offset, uint16_t PageLevel2Offset);

// Defined in NASM Source file

extern "C" void x86_64_FlushTLB();
extern "C" void x86_64_LoadCR3(uint64_t value, uint64_t DEBUG);
extern "C" uint64_t x86_64_GetCR3();
extern "C" uint64_t x86_64_GetCR2();

extern "C" bool x86_64_EnsureNX();

#endif /* _KERNEL_X86_64_PAGING_UTIL_HPP */