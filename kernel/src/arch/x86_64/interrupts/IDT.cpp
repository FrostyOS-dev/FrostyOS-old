/*
Copyright (©) 2022-2024  Frosty515

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "IDT.hpp"

#include <util.h>

IDT idt = {};

void SetOffset(IDTDescEntry* entry, uint64_t offset) {
    entry->offset0 = (uint16_t) (offset & 0xffff);
    entry->offset1 = (uint16_t)((offset & 0xffff0000) >> 16);
    entry->offset2 = (uint32_t)((offset & 0xffffffff00000000) >> 32);
}

uint64_t GetOffset(IDTDescEntry* entry) {
    uint64_t offset = 0;
    offset |= (uint64_t)entry->offset0;
    offset |= (uint64_t)entry->offset1 << 16;
    offset |= (uint64_t)entry->offset2 << 32;
    return offset;
}

void x86_64_IDT_Load(IDTR* idtr) {
    asm volatile("lidt %0" : : "m" (*idtr));
}

void x86_64_IDT_Initialize() {
    idt.idtr.Limit = sizeof(idt.entries) - 1;
    idt.idtr.Offset = (uintptr_t) &idt.entries[0];
    memset(&idt.entries[0], 0, sizeof(idt.entries));
}

void x86_64_IDT_DisableGate(uint8_t interrupt) {
    FLAG_UNSET(idt.entries[interrupt].type_attr, IDT_FLAG_PRESENT);
}

void x86_64_IDT_EnableGate(uint8_t interrupt) {
    FLAG_SET(idt.entries[interrupt].type_attr, IDT_FLAG_PRESENT);
}

void x86_64_IDT_SetGate(uint8_t interrupt, void* base, uint16_t segmentDescriptor, uint8_t flags) {
    idt.entries[interrupt].offset0 = (uint16_t) (((uint64_t)base) & 0xffff);
    idt.entries[interrupt].selector = segmentDescriptor;
    idt.entries[interrupt].ist = 0;
    idt.entries[interrupt].type_attr = flags;
    idt.entries[interrupt].offset1 = (uint16_t)((((uint64_t)base) & 0xffff0000) >> 16);
    idt.entries[interrupt].offset2 = (uint32_t)((((uint64_t)base) & 0xffffffff00000000) >> 32);
    idt.entries[interrupt].ignore = 0;
}
