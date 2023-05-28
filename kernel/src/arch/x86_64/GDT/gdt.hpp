#ifndef _GDT_HPP
#define _GDT_HPP

#include <stdint.h>

struct GDTDescriptor {
    uint16_t Size;
    uint64_t Offset;
} __attribute__((packed));

struct GDTEntry {
    uint16_t Limit0;
    uint16_t Base0;
    uint8_t Base1;
    uint8_t AccessByte;
    uint8_t Limit1_Flags; // upper half is flags and lower half is Limit1
    uint8_t Base2;
} __attribute__((packed));

struct GDT {
    GDTEntry Null;
    GDTEntry KernelCode;
    GDTEntry KernelData;
    GDTEntry UserNull;
    GDTEntry UserCode;
    GDTEntry UserData;
} __attribute__((packed)) __attribute__((aligned(0x1000)));

extern GDT DefaultGDT;

extern "C" void x86_64_LoadGDT(GDTDescriptor* gdtDescriptor);

#endif /* _GDT_HPP */