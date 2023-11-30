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

#include "FileDescriptorManager.hpp"

#include <stdlib.h>
#include <util.h>


FileDescriptorManager* g_KFDManager = nullptr;


FileDescriptorManager::FileDescriptorManager() : m_auto_expand(true) {

}

FileDescriptorManager::FileDescriptorManager(uint8_t* buffer, size_t max_size) : m_auto_expand(false), m_bitmap(buffer, ALIGN_UP(max_size, 8)) { // size must be divisible by 8
    fast_memset(buffer, 0, max_size >> 3);
}

FileDescriptorManager::~FileDescriptorManager() {
    for (uint64_t i = 0; i < m_descriptors.getCount(); i++)
        m_descriptors.remove(UINT64_C(0));
}

bool FileDescriptorManager::ReserveFileDescriptor(FileDescriptorType type, void* data, FileDescriptorMode mode, fd_t ID) {
    if ((long)(m_bitmap.GetSize() << 3) > ID && m_bitmap[ID])
        return false;
    if (!ExpandBitmap(ALIGN_UP((ID+1), 8)))
        return false;
    FileDescriptor* descriptor = new FileDescriptor(type, data, mode, ID);
    if (descriptor == nullptr)
        return false;
    if (descriptor->GetLastError() != FileDescriptorError::SUCCESS) {
        delete descriptor;
        return false;
    }
    m_descriptors.insert(descriptor);
    m_bitmap.Set(ID, true);
    return true;
}

bool FileDescriptorManager::ReserveFileDescriptor(FileDescriptor* descriptor) {
    if (descriptor == nullptr)
        return false;
    fd_t ID = descriptor->GetID();
    if ((long)(m_bitmap.GetSize() << 3) > ID && m_bitmap[ID])
        return false;
    if (!ExpandBitmap(ALIGN_UP((ID+1), 8)))
        return false;
    m_descriptors.insert(descriptor);
    m_bitmap.Set(ID, true);
    return true;
}

bool FileDescriptorManager::UnreserveFileDescriptor(fd_t ID) {
    FileDescriptor* descriptor = GetFileDescriptor(ID);
    if (descriptor == nullptr)
        return false;
    m_descriptors.remove(descriptor);
    m_bitmap.Set(ID, 0);
    return true;
}

FileDescriptor* FileDescriptorManager::GetFileDescriptor(fd_t ID) const {
    if (ID >= (long)(m_bitmap.GetSize() << 3))
        return nullptr;
    for (uint64_t i = 0; i < m_descriptors.getCount(); i++) {
        FileDescriptor* descriptor = m_descriptors.get(i);
        if (descriptor == nullptr)
            return nullptr; // should never happen
        if (descriptor->GetID() == ID)
            return descriptor;
    }
    return nullptr;
}

fd_t FileDescriptorManager::AllocateFileDescriptor(FileDescriptorType type, void* data, FileDescriptorMode mode) {
    fd_t ID;
    bool found = false;
    size_t size = m_bitmap.GetSize();
    for (uint64_t i = 0; i < (m_bitmap.GetSize() << 3); i++) {
        if (!m_bitmap[i]) {
            ID = i;
            found = true;
            break;
        }
    }
    if (!found) {
        if (!ExpandBitmap(ALIGN_UP((size + 1), 8)))
            return -1;
        ID = size;
        found = true;
    }
    FileDescriptor* descriptor = new FileDescriptor(type, data, mode, ID);
    if (descriptor == nullptr)
        return -1;
    if (descriptor->GetLastError() != FileDescriptorError::SUCCESS) {
        delete descriptor;
        return -1;
    }
    m_descriptors.insert(descriptor);
    m_bitmap.Set(ID, 1);
    return ID;
}

bool FileDescriptorManager::FreeFileDescriptor(fd_t ID) {
    FileDescriptor* descriptor = GetFileDescriptor(ID);
    if (descriptor == nullptr)
        return false;
    m_descriptors.remove(descriptor);
    m_bitmap.Set(ID, 0);
    return true;
}

bool FileDescriptorManager::ExpandBitmap(size_t new_size) {
    size_t old_size = m_bitmap.GetSize();
    if (new_size <= old_size)
        return true; // already big enough, no need to expand
    if (!m_auto_expand)
        return false; // cannot expand
    void* data = krealloc(m_bitmap.GetBuffer(), new_size);
    if (data == nullptr)
        return false;
    fast_memset((void*)((uint64_t)data + old_size), 0, (new_size - old_size) >> 3);
    m_bitmap.SetBuffer((uint8_t*)data);
    m_bitmap.SetSize(new_size);
    return true;
}