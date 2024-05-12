/*
Copyright (©) 2023-2024  Frosty515

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

#ifndef _FILE_DESCRIPTOR_MANAGER_HPP
#define _FILE_DESCRIPTOR_MANAGER_HPP

#include "FileDescriptor.hpp"

#include <Data-structures/LinkedList.hpp>
#include <Data-structures/Bitmap.hpp>

#include <stddef.h>
#include <stdint.h>
#include <spinlock.h>

class FileDescriptorManager {
public:
    FileDescriptorManager();
    FileDescriptorManager(uint8_t* buffer, size_t max_size);
    ~FileDescriptorManager();

    bool ReserveFileDescriptor(FileDescriptorType type, void* data, FileDescriptorMode mode, fd_t ID);
    bool ReserveFileDescriptor(FileDescriptor* descriptor);
    bool UnreserveFileDescriptor(fd_t ID);

    FileDescriptor* GetFileDescriptor(fd_t ID) const;

    fd_t AllocateFileDescriptor(FileDescriptorType type, void* data, FileDescriptorMode mode);
    bool FreeFileDescriptor(fd_t ID);

    void ForceUnlock(); // should only ever be used in a PANIC to get emergency access to resources.

private:
    bool ExpandBitmap(size_t new_size);

private:
    bool m_auto_expand;
    LinkedList::SimpleLinkedList<FileDescriptor> m_descriptors;
    Bitmap m_bitmap;
    mutable spinlock_t m_lock;
};

extern FileDescriptorManager* g_KFDManager;

#endif /* _FILE_DESCRIPTOR_MANAGER_HPP */