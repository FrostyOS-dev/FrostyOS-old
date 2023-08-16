/*
Copyright (©) 2023  Frosty515

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

#ifndef _INITRAMFS_HPP
#define _INITRAMFS_HPP

#include <stdint.h>
#include <stddef.h>

bool Initialise_InitRAMFS(void* address, size_t size);

namespace TarFS {
    struct USTARItemHeader {
        char filepath[100];
        char mode[8];
        char uid[8];
        char gid[8];
        char size[12];
        char mtime[12];
        char Checksum[8];
        char TypeFlag;
        char filename[100];
        char ID[6]; // should be "ustar", with null termination
        char version[2]; // should be "00", with no null termination
        char OwnerUserName[32];
        char OwnerGroupName[32];
        char DeviceMajorNumber[8];
        char DeviceMinorNumber[8];
        char FilenamePrefix[155];
    } __attribute__((packed));

    enum class USTARItemType {
        FILE = 0,
        HARD_LINK = 1,
        SYM_LINK = 2,
        CHAR_DEVICE = 3,
        BLOCK_DEVICE = 4,
        DIRECTORY = 5,
        NAMED_PIPE = 6
    };

    size_t ASCII_OCT_To_UInt(char* str, size_t len);

    size_t USTAR_Lookup(uint8_t* archive, const char* filename, uint8_t** out);
    void EnumerateUSTAR(uint8_t* archive);
}

#endif /* _INITRAMFS_HPP */