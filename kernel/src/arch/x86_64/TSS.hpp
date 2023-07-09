#ifndef _X86_64_TSS_HPP
#define _X86_64_TSS_HPP

#include <stdint.h>

struct x86_64_TSS {
    uint32_t Reserved0;
    uint64_t RSP[3];
    uint64_t Reserved1;
    uint64_t IST[7];
    uint64_t Reserved2;
    uint16_t Reserved3;
    uint16_t IOPB;
} __attribute__((packed));

void x86_64_TSS_Init();

extern "C" void x86_64_TSS_Load(uint16_t GDTOffset);

#endif /* X86_64_TSS_HPP */