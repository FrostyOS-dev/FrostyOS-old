/*
Copyright (©) 2024  Frosty515

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

#include "PAT.hpp"

#include "../MSR.h"

#define PAT_MSR 0x277

struct PAT {
    struct PATEntry {
        uint8_t Type : 3;
        uint8_t Reserved : 5;
    } __attribute__((packed)) Entries[8];
} __attribute__((packed));

void x86_64_InitPAT() {
    uint64_t raw_value = x86_64_ReadMSR(PAT_MSR);
    PAT pat = *(PAT*)&raw_value;
    pat.Entries[0].Type = (int)x86_64_PATType::WriteBack;
    pat.Entries[1].Type = (int)x86_64_PATType::WriteThrough;
    pat.Entries[2].Type = (int)x86_64_PATType::Uncached;
    pat.Entries[3].Type = (int)x86_64_PATType::Uncacheable;
    pat.Entries[4].Type = (int)x86_64_PATType::WriteProtected;
    pat.Entries[5].Type = (int)x86_64_PATType::WriteCombining;
    pat.Entries[6].Type = (int)x86_64_PATType::WriteBack;
    pat.Entries[7].Type = (int)x86_64_PATType::WriteBack;
    x86_64_WriteMSR(PAT_MSR, *(uint64_t*)&pat);
}

uint16_t x86_64_GetPTEFlagsFromPAT(x86_64_PATType type) {
    switch (type) {
        case x86_64_PATType::Uncacheable:
            return (1 << 3) | (1 << 4);
        case x86_64_PATType::WriteCombining:
            return (1 << 3) | (1 << 7);
        case x86_64_PATType::WriteThrough:
            return 1 << 3;
        case x86_64_PATType::WriteProtected:
            return 1 << 7;
        case x86_64_PATType::WriteBack:
            return 0;
        case x86_64_PATType::Uncached:
            return 1 << 4;
        default:
            return 0; // default to write-back
    }
}

uint16_t x86_64_GetPDEFlagsFromPAT(x86_64_PATType type) {
    switch (type) {
        case x86_64_PATType::Uncacheable:
            return (1 << 3) | (1 << 4);
        case x86_64_PATType::WriteCombining:
            return (1 << 3) | (1 << 12);
        case x86_64_PATType::WriteThrough:
            return 1 << 3;
        case x86_64_PATType::WriteProtected:
            return 1 << 12;
        case x86_64_PATType::WriteBack:
            return 0;
        case x86_64_PATType::Uncached:
            return 1 << 4;
        default:
            return 0; // default to write-back
    }
}
