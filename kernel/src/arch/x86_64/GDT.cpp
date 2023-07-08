#include "GDT.hpp"

#include <util.h>

x86_64_GDTEntry __attribute__((aligned(0x1000))) g_GDT[6];

void x86_64_GDTInit() {
    fast_memset(&(g_GDT[0]), 0, sizeof(x86_64_GDTEntry) * 6 / 8);
    g_GDT[1].AccessByte = ((uint8_t)x86_64_GDTAccessByte::Read | (uint8_t)x86_64_GDTAccessByte::Executable | (uint8_t)x86_64_GDTAccessByte::CodeData | (uint8_t)x86_64_GDTAccessByte::Ring0 | (uint8_t)x86_64_GDTAccessByte::Present);
    g_GDT[1].Flags = (uint8_t)x86_64_GDTFlags::LongMode | (uint8_t)x86_64_GDTFlags::PageBlocks;
    g_GDT[2].AccessByte = ((uint8_t)x86_64_GDTAccessByte::Write | (uint8_t)x86_64_GDTAccessByte::CodeData | (uint8_t)x86_64_GDTAccessByte::Ring0 | (uint8_t)x86_64_GDTAccessByte::Present);
    g_GDT[2].Flags = (uint8_t)x86_64_GDTFlags::LongMode | (uint8_t)x86_64_GDTFlags::PageBlocks;

    g_GDT[3].AccessByte = ((uint8_t)x86_64_GDTAccessByte::Read | (uint8_t)x86_64_GDTAccessByte::Executable | (uint8_t)x86_64_GDTAccessByte::CodeData | (uint8_t)x86_64_GDTAccessByte::Ring3 | (uint8_t)x86_64_GDTAccessByte::Present);
    g_GDT[3].Flags = (uint8_t)x86_64_GDTFlags::LongMode | (uint8_t)x86_64_GDTFlags::PageBlocks;
    g_GDT[4].AccessByte = ((uint8_t)x86_64_GDTAccessByte::Write | (uint8_t)x86_64_GDTAccessByte::CodeData | (uint8_t)x86_64_GDTAccessByte::Ring3 | (uint8_t)x86_64_GDTAccessByte::Present);
    g_GDT[4].Flags = (uint8_t)x86_64_GDTFlags::LongMode | (uint8_t)x86_64_GDTFlags::PageBlocks;

    x86_64_GDTDescriptor GDTDescriptor;
    GDTDescriptor.Size = sizeof(x86_64_GDTEntry) * 6 - 1;
    GDTDescriptor.Offset = (uint64_t)&(g_GDT[0]);
    x86_64_LoadGDT(&GDTDescriptor);
}
