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
#include <stdio.hpp>
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
        fprintf(VFS_DEBUG, "initramfs item: path=\"%s\", size=%lu, type=%c\n", header->filepath, size, header->TypeFlag);

        uint8_t last_separator = 255;
        for (uint8_t i = 0; i < 99 && header->filepath[i]; i++) {
            if (header->filepath[i] == PATH_SEPARATOR && header->filepath[i + 1] != 0)
                last_separator = i;
        }
        char* parent = "/";
        char* name = header->filepath;
        if (last_separator < 99) {
            parent = new char[last_separator + 1];
            assert(parent != nullptr);
            memcpy((void*)((uint64_t)parent + 0), header->filepath, last_separator);
            name = &(header->filepath[last_separator + 1]);
        }

        assert(g_VFS != nullptr);

        switch (header->TypeFlag - '0') {
        case 0: // File
            {
            assert(g_VFS->CreateFile(parent, name, size));
            FileStream* stream = g_VFS->OpenStream(header->filepath, VFS_READ | VFS_WRITE);
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
        case 5: // Folder
            assert(g_VFS->CreateFolder(parent, name));
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

    int USTAR_Lookup(uint8_t* archive, char* filename, uint8_t** out) {
        uint8_t* ptr = archive;
 
        while (memcmp(ptr + 257, "ustar", 5) == 0) {
            int filesize = ASCII_OCT_To_UInt((char*)((uint64_t)ptr + 0x7c), 12);
            if (memcmp(ptr, filename, strlen(filename) + 1) == 0) {
                *out = (uint8_t*)((uint64_t)ptr + 512);
                return filesize;
            }
            ptr = (uint8_t*)((uint64_t)ptr + (((filesize + 511) / 512) + 1) * 512);
        }
        return 0;
    }

    void EnumerateUSTAR(uint8_t* archive) {
        USTARItemHeader* header = (USTARItemHeader*)archive;
 
        while (memcmp(&(header->ID), "ustar", 5) == 0) {
            
            uint64_t size = ASCII_OCT_To_UInt(header->size, 12);
            fprintf(VFS_DEBUG, "USTAR item: path=\"%s\", size=%lu, type=%c\n", header->filepath, size, header->TypeFlag);

            if ((header->TypeFlag - '0') == 0) {
                fputs(VFS_DEBUG, "File contents:\n\n");
                fwrite((const void*)((uint64_t)header + 512), 1, size, VFS_DEBUG);
                fputs(VFS_DEBUG, "\n\n");
            }

            header = (USTARItemHeader*)((uint64_t)header + 512 + ALIGN_UP(size, 512));
        }
    }
}