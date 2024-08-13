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

#include "FileDescriptor.hpp"
#include "DirectoryStream.hpp"
#include "FileStream.hpp"

#include "TempFS/TempFSInode.hpp"

#include <cstdint>
#include <errno.h>
#include <spinlock.h>
#include <util.h>

#include <file.h>

#include <HAL/hal.hpp>

#ifdef __x86_64__
#include <arch/x86_64/E9.h>
#endif

FileDescriptor::FileDescriptor() : m_TTYInfo(nullptr), m_Stream(nullptr), m_is_open(false), m_type(FileDescriptorType::UNKNOWN), m_mode(FileDescriptorMode::READ), m_ID(-1), m_init_successful(false), m_lock(0) {

}

FileDescriptor::FileDescriptor(FileDescriptorType type, void* data, FileDescriptorMode mode, fd_t ID) : m_TTYInfo(nullptr), m_Stream(nullptr), m_is_open(false), m_type(type), m_mode(mode), m_ID(ID), m_init_successful(false), m_lock(0) {
    spinlock_acquire(&m_lock);
    switch (m_type) {
    case FileDescriptorType::FILE_STREAM:
        m_Stream = data;
        break;
    case FileDescriptorType::DIRECTORY_STREAM:
        switch (m_mode) {
        case FileDescriptorMode::READ:
            m_Stream = data;
            break;
        case FileDescriptorMode::WRITE:
        case FileDescriptorMode::APPEND:
        case FileDescriptorMode::READ_WRITE:
            spinlock_release(&m_lock);
            return;
        }
        break;
    case FileDescriptorType::TTY:
        switch (m_mode) {
        case FileDescriptorMode::READ_WRITE:
        case FileDescriptorMode::WRITE:
            m_TTYInfo = (TTYFileDescriptor*)data;
            m_TTYInfo->tty->seek(0, m_TTYInfo->mode);
            break;
        case FileDescriptorMode::READ:
        case FileDescriptorMode::APPEND:
            m_TTYInfo = (TTYFileDescriptor*)data;
            break;
        default:
            spinlock_release(&m_lock);
            return;
        }
        m_is_open = true;
        break;
    default:
        spinlock_release(&m_lock);
        return;
    }
    m_init_successful = true;
    spinlock_release(&m_lock);
}

FileDescriptor::~FileDescriptor() {
    if (m_is_open)
        (void)Close(); // return value is irrelevant
}

int FileDescriptor::Open() {
    spinlock_acquire(&m_lock);
    switch (m_type) {
    case FileDescriptorType::TTY:
        spinlock_release(&m_lock);
        m_is_open = true;
        return ESUCCESS; // already open
    case FileDescriptorType::FILE_STREAM: {
        FileStream* fileStream = (FileStream*)m_Stream;
        int rc = fileStream->Open();
        if (rc != ESUCCESS) {
            spinlock_release(&m_lock);
            return rc;
        }
        if (m_mode == FileDescriptorMode::APPEND) {
            if (fileStream->Seek(fileStream->GetSize() - 1) != ESUCCESS) { // seek to end
                spinlock_release(&m_lock);
                return -ENOSYS; // should NEVER happen
            }
        }
        m_is_open = true;
        spinlock_release(&m_lock);
        return ESUCCESS;
    }
    case FileDescriptorType::DIRECTORY_STREAM: {
        DirectoryStream* directoryStream = (DirectoryStream*)m_Stream;
        int rc = directoryStream->Open();
        if (rc != ESUCCESS) {
            spinlock_release(&m_lock);
            return rc;
        }
        m_is_open = true;
        spinlock_release(&m_lock);
        return ESUCCESS;
    }
    default:
        return -ENOSYS; // should NEVER happen
    }
}

int FileDescriptor::Close() {
    spinlock_acquire(&m_lock);
    switch (m_type) {
    case FileDescriptorType::TTY:
        m_is_open = false;
        spinlock_release(&m_lock);
        return ESUCCESS; // cannot be closed
    case FileDescriptorType::FILE_STREAM: {
        FileStream* fileStream = (FileStream*)m_Stream;
        if (fileStream->Close() != ESUCCESS) {
            spinlock_release(&m_lock);
            return -ENOSYS; // should NEVER happen
        }
        m_is_open = false;
        spinlock_release(&m_lock);
        return ESUCCESS;
    }
    case FileDescriptorType::DIRECTORY_STREAM: {
        DirectoryStream* directoryStream = (DirectoryStream*)m_Stream;
        if (directoryStream->Close() != ESUCCESS) {
            spinlock_release(&m_lock);
            return -ENOSYS; // should NEVER happen
        }
        m_is_open = false;
        spinlock_release(&m_lock);
        return ESUCCESS;
    }
    default:
        spinlock_release(&m_lock);
        return -ENOSYS; // should NEVER happen
    }
}


int64_t FileDescriptor::Read(uint8_t* buffer, int64_t count, int* status) {
    spinlock_acquire(&m_lock);
    switch (m_mode) {
    case FileDescriptorMode::READ:
    case FileDescriptorMode::READ_WRITE:
        break;
    default:
        spinlock_release(&m_lock);
        return -EBADF;
    }
    switch (m_type) {
    case FileDescriptorType::TTY: {
        int64_t i = 0;
        while (i < count) {
            int c = m_TTYInfo->tty->getc(m_TTYInfo->mode);
            if (c == EOF)
                break;
            buffer[i] = (uint8_t)c;
            i++;
        }
        spinlock_release(&m_lock);
        if (status != nullptr) {
            if (i < count)
                *status = -EAGAIN;
            else
                *status = ESUCCESS;
        }
        return i; // NOTE: partial reads from TTYs mean the keyboard device had an error or was disconnected.
    }
    case FileDescriptorType::FILE_STREAM: {
        FileStream* fileStream = (FileStream*)m_Stream;
        int i_status = 0;
        int64_t rc = fileStream->ReadStream(buffer, count, &i_status);
        if (rc >= 0 && status != nullptr)
            *status = i_status;
        return rc;
    }
    case FileDescriptorType::DIRECTORY_STREAM: {
        DirectoryStream* directoryStream = (DirectoryStream*)m_Stream;
        int i_status = 0;
        Inode* inode = directoryStream->GetNextInode(&i_status);
        if (inode == nullptr) {
            spinlock_release(&m_lock);
            return (int64_t)i_status;
        }
        size_t inode_size = 0;
        // switch for the file system type
        switch (directoryStream->GetFileSystemType()) {
        case FileSystemType::TMPFS:
            inode_size = sizeof(TempFS::TempFSInode);
            break;
        default:
            spinlock_release(&m_lock);
            return -ENOSYS;
        }
        if ((size_t)count != inode_size) {
            spinlock_release(&m_lock);
            return -EINVAL;
        }
        memcpy(buffer, (void*)inode, inode_size);
        spinlock_release(&m_lock);
        if (status != nullptr)
            *status = ESUCCESS;
        return inode_size;
    }
    default:
        return -ENOSYS;
    }
}

int64_t FileDescriptor::Write(const uint8_t* buffer, int64_t count, int* status) {
    spinlock_acquire(&m_lock);
    switch (m_mode) {
    case FileDescriptorMode::APPEND:
    case FileDescriptorMode::WRITE:
    case FileDescriptorMode::READ_WRITE:
        break;
    default:
        spinlock_release(&m_lock);
        return -EBADF;
    }
    switch (m_type) {
    case FileDescriptorType::TTY: {
        int64_t i = 0;
        while (i < count) {
            m_TTYInfo->tty->putc(buffer[i], m_TTYInfo->mode);
            i++;
        }
        spinlock_release(&m_lock);
        if (status != nullptr) {
            if (i < count)
                *status = -EAGAIN;
            else
                *status = ESUCCESS;
        }
        return count;
    }
    case FileDescriptorType::FILE_STREAM: {
        FileStream* fileStream = (FileStream*)m_Stream;
        int i_status = 0;
        int64_t rc = fileStream->WriteStream(buffer, count, &i_status);
        spinlock_release(&m_lock);
        if (rc >= 0 && status != nullptr)
            *status = i_status;
        return rc;
    }
    case FileDescriptorType::DIRECTORY_STREAM:
        spinlock_release(&m_lock);
        return -EACCES;
    default:
        spinlock_release(&m_lock);
        return -ENOSYS;
    }
}

int FileDescriptor::Seek(int64_t offset) {
    if (offset < 0)
        return -EINVAL;
    spinlock_acquire(&m_lock);
    switch (m_mode) {
    case FileDescriptorMode::READ:
    case FileDescriptorMode::WRITE:
    case FileDescriptorMode::READ_WRITE:
        break;
    default:
        spinlock_release(&m_lock);
        return -EBADF;
    }
    switch (m_type) {
    case FileDescriptorType::TTY: {
        m_TTYInfo->tty->seek(offset, m_TTYInfo->mode);
        spinlock_release(&m_lock);
        return ESUCCESS;
    }
    case FileDescriptorType::FILE_STREAM: {
        FileStream* fileStream = (FileStream*)m_Stream;
        int rc = fileStream->Seek(offset);
        spinlock_release(&m_lock);
        return rc;
    }
    case FileDescriptorType::DIRECTORY_STREAM: {
        DirectoryStream* directoryStream = (DirectoryStream*)m_Stream;
        int rc = directoryStream->Seek(offset);
        spinlock_release(&m_lock);
        return rc;
    }
    default:
        return -ENOSYS;
    }
}

int FileDescriptor::Rewind() {
    spinlock_acquire(&m_lock);
    switch (m_mode) {
    case FileDescriptorMode::READ:
    case FileDescriptorMode::WRITE:
    case FileDescriptorMode::READ_WRITE:
        break;
    default:
        spinlock_release(&m_lock);
        return -EBADF;
    }
    switch (m_type) {
    case FileDescriptorType::TTY:
        m_TTYInfo->tty->seek(0, m_TTYInfo->mode);
        spinlock_release(&m_lock);
        return ESUCCESS;
    case FileDescriptorType::FILE_STREAM: {
        FileStream* fileStream = (FileStream*)m_Stream;
        int rc = fileStream->Rewind();
        spinlock_release(&m_lock);
        return rc;
    }
    case FileDescriptorType::DIRECTORY_STREAM: {
        DirectoryStream* directoryStream = (DirectoryStream*)m_Stream;
        int rc = directoryStream->Seek(0);
        spinlock_release(&m_lock);
        return rc;
    }
    default:
        spinlock_release(&m_lock);
        return -ENOSYS;
    }
}

fd_t FileDescriptor::GetID() const {
    return m_ID;
}

void* FileDescriptor::GetData() const {
    switch (m_type) {
    case FileDescriptorType::FILE_STREAM:
    case FileDescriptorType::DIRECTORY_STREAM:
        return m_Stream;
    case FileDescriptorType::TTY:
        return m_TTYInfo;
    default:
        return nullptr;
    }
}

FileDescriptorType FileDescriptor::GetType() const {
    return m_type;
}

bool FileDescriptor::WasInitSuccessful() const {
    return m_init_successful;
}

void FileDescriptor::ForceUnlock() {
    spinlock_release(&m_lock);
    /*if (m_type == FileDescriptorType::TTY)
        m_TTYInfo->tty->Unlock();*/
}
