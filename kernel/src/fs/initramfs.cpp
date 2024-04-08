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

#include "initramfs.hpp"
#include "VFS.hpp"
#include "FileStream.hpp"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <util.h>

bool Initialise_InitRAMFS(void* address, size_t size) {
    if (g_VFS == nullptr)
        return false;

    using namespace TarFS;
    USTARItemHeader* header = (USTARItemHeader*)address;


    while (memcmp(&(header->ID), "ustar", 5) == 0 && ((uint64_t)header - (uint64_t)address) < size) {
        assert(header->filepath[99] == 0);
        uint64_t size = ASCII_OCT_To_UInt(header->size, 12);
        uint32_t uid = (uint32_t)ASCII_OCT_To_UInt(header->uid, 8);
        uint32_t gid = (uint32_t)ASCII_OCT_To_UInt(header->gid, 8);
        uint16_t ACL = (uint16_t)ASCII_OCT_To_UInt(header->mode, 8);
        dbgprintf("initramfs item: path=\"%s\", size=%lu, type=%c, uid=%u, gid=%u, ACL=%03ho\n", header->filepath, size, header->TypeFlag, uid, gid, ACL);

        uint8_t last_separator = 255;
        for (uint8_t i = 0; i < 99 && header->filepath[i]; i++) {
            if (header->filepath[i] == PATH_SEPARATOR && header->filepath[i + 1] != 0)
                last_separator = i;
        }
        char const* parent = "/";
        char* name = header->filepath;
        if (last_separator < 99) {
            parent = new char[last_separator + 1];
            assert(parent != nullptr);
            memcpy((void*)((uint64_t)parent + 0), header->filepath, last_separator);
            name = &(header->filepath[last_separator + 1]);
        }

        assert(g_VFS != nullptr);

        FilePrivilegeLevel privilege = {uid, gid, ACL};
        
        switch (header->TypeFlag - '0') {
        case 0: // File
            {
            assert(g_VFS->CreateFile({0, 0, 07777}, parent, name, size, false, privilege));
            if (size == 0)
                break;
            FileStream* stream = g_VFS->OpenStream({0, 0, 07777}, header->filepath, VFS_READ | VFS_WRITE);
            assert(stream != nullptr);
            assert(stream->Open());
            assert(stream->WriteStream((const uint8_t*)((uint64_t)header + 512), size));

            // Validate the data. can be excluded

            uint8_t* buffer = new uint8_t[size];
            assert(stream->Rewind());
            assert(stream->ReadStream(buffer, size));
            assert(memcmp(buffer, (const void*)((uint64_t)header + 512), size) == 0);


            assert(stream->Close());
            assert(g_VFS->CloseStream(stream));
            }
            break;
        case 2: // Symbolic Link
            assert(g_VFS->CreateSymLink({0, 0, 07777}, parent, name, header->filename, false, privilege));
            break;
        case 5: // Folder
            assert(g_VFS->CreateFolder({0, 0, 07777}, parent, name, false, privilege));
            break;
        default:
            assert(false);
            break;
        }

        header = (USTARItemHeader*)((uint64_t)header + 512 + ALIGN_UP(size, 512));
    }
    return true;
}

namespace TarFS {
    size_t ASCII_OCT_To_UInt(char* str, size_t len) {
        if (str == nullptr)
            return (size_t)-1;
        size_t num = 0;
        for (size_t i = 0; i < len; i++) {
            if (str[i] == 0)
                break;
            num *= 8;
            num += str[i] - '0';
        }
        return num;
    }

    size_t USTAR_Lookup(uint8_t* archive, const char* filename, uint8_t** out) {
        USTARItemHeader* header = (USTARItemHeader*)archive;
 
        while (memcmp(&(header->ID), "ustar", 5) == 0) {
            
            uint64_t size = ASCII_OCT_To_UInt(header->size, 12);

            if (memcmp(header->filepath, filename, strlen(filename) + 1) == 0) {
                if (size > 0)
                    *out = (uint8_t*)((uint64_t)header + 512);
                return size;
            }

            header = (USTARItemHeader*)((uint64_t)header + 512 + ALIGN_UP(size, 512));
        }
        return 0;
    }

    void EnumerateUSTAR(uint8_t* archive) {
        USTARItemHeader* header = (USTARItemHeader*)archive;
 
        while (memcmp(&(header->ID), "ustar", 5) == 0) {
            
            uint64_t size = ASCII_OCT_To_UInt(header->size, 12);
            dbgprintf("USTAR item: path=\"%s\", size=%lu, type=%c\n", header->filepath, size, header->TypeFlag);

            if ((header->TypeFlag - '0') == 0) {
                dbgputs("File contents:\n\n");
                fwrite((const void*)((uint64_t)header + 512), 1, size, stddebug);
                dbgputs("\n\n");
            }

            header = (USTARItemHeader*)((uint64_t)header + 512 + ALIGN_UP(size, 512));
        }
    }
}