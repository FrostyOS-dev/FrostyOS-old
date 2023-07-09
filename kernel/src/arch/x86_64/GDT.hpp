#ifndef _X86_64_GDT_HPP
#define _X86_64_GDT_HPP

#include <stdint.h>

#include "TSS.hpp"

struct x86_64_GDTDescriptor {
    uint16_t Size;
    uint64_t Offset;
} __attribute__((packed));

enum class x86_64_GDTAccessByte {
    Accessed = 1,
    Write = 2, // only applicable to data selectors
    Read = 2, // only applicable to code selectors
    GrowDown = 4, // only applicable to data selectors
    EqualOrLower = 4, // only applicable to code selectors
    Executable = 8,
    CodeData = 16,
    Ring0 = 0,
    Ring1 = 32,
    Ring2 = 64,
    Ring3 = 96,
    Present = 128
};

enum class x86_64_GDTSysSegType {
    LDT = 0x2,
    TSS_AVAILABLE = 0x9,
    TSS_BUSY = 0xB
};

enum class x86_64_GDTFlags {
    LongMode = 2,
    PageBlocks = 8
};

struct x86_64_GDTEntry {
    uint16_t Limit0;
    uint16_t Base0;
    uint8_t Base1;
    uint8_t AccessByte;
    uint8_t Limit1 : 4;
    uint8_t Flags : 4;
    uint8_t Base2;
} __attribute__((packed));

struct x86_64_GDTSysEntry {
    uint16_t Limit0;
    uint16_t Base0;
    uint8_t Base1;
    uint8_t AccessByte;
    uint8_t Limit1 : 4;
    uint8_t Flags : 4;
    uint8_t Base2;
    uint32_t Base3;
    uint32_t Reserved;
} __attribute__((packed));

void x86_64_GDT_SetTSS(x86_64_TSS* TSS);

void x86_64_GDTInit();

extern "C" void x86_64_LoadGDT(x86_64_GDTDescriptor* gdtDescriptor);

#endif /* _X86_64_GDT_HPP */