#ifndef _KERNEL_IDT_HPP
#define _KERNEL_IDT_HPP

#include <stddef.h>
#include <stdint.h>

#include <util.h>

enum IDT_FLAGS {
    IDT_FLAG_GATE_TASK              = 0x5,
    IDT_FLAG_GATE_64BIT_INT         = 0xE,
    IDT_FLAG_GATE_64BIT_TRAP        = 0xF,

    IDT_FLAG_RING0                  = (0 << 5),
    IDT_FLAG_RING1                  = (1 << 5),
    IDT_FLAG_RING2                  = (2 << 5),
    IDT_FLAG_RING3                  = (3 << 5),

    IDT_FLAG_PRESENT                = 0x80,
};

struct IDTDescEntry {
    uint16_t offset0;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset1;
    uint32_t offset2;
    uint32_t ignore;
};

void SetOffset(IDTDescEntry* entry, uint64_t offset);
uint64_t GetOffset(IDTDescEntry* entry);

struct IDTR {
    uint16_t Limit;
    uint64_t Offset;
} __attribute__((packed));

struct IDT {
    IDTDescEntry entries[256];
    IDTR idtr;
};

extern IDT idt;

void x86_64_IDT_Load(IDTR* idtr);
void x86_64_IDT_Initialize();
void x86_64_IDT_DisableGate(uint8_t interrupt);
void x86_64_IDT_EnableGate(uint8_t interrupt);
void x86_64_IDT_SetGate(uint8_t interrupt, void* base, uint16_t segmentDescriptor, uint8_t flags);

/*

IVT Offset | INT #     | Description
-----------+-----------+-----------------------------------
0x0000     | 0x00      | Divide by 0
0x0004     | 0x01      | Reserved
0x0008     | 0x02      | NMI Interrupt
0x000C     | 0x03      | Breakpoint (INT3)
0x0010     | 0x04      | Overflow (INTO)
0x0014     | 0x05      | Bounds range exceeded (BOUND)
0x0018     | 0x06      | Invalid opcode (UD2)
0x001C     | 0x07      | Device not available (WAIT/FWAIT)
0x0020     | 0x08      | Double fault
0x0024     | 0x09      | Coprocessor segment overrun
0x0028     | 0x0A      | Invalid TSS
0x002C     | 0x0B      | Segment not present
0x0030     | 0x0C      | Stack-segment fault
0x0034     | 0x0D      | General protection fault
0x0038     | 0x0E      | Page fault
0x003C     | 0x0F      | Reserved
0x0040     | 0x10      | x87 FPU error
0x0044     | 0x11      | Alignment check
0x0048     | 0x12      | Machine check
0x004C     | 0x13      | SIMD Floating-Point Exception
0x00xx     | 0x14-0x1F | Reserved
0x0xxx     | 0x20-0xFF | User definable

*/

#endif /* _KERNEL_IDT_HPP */