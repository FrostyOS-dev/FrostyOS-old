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

#include "FileStream.hpp"

#include "TempFS/TempFSInode.hpp"
#include "spinlock.h"

#include <errno.h>

FileStream::FileStream(Inode* inode, VFS_MountPoint* mountPoint, uint8_t modes, FilePrivilegeLevel privilege) : m_open(false), m_inode(inode), m_inode_state(nullptr), m_mountPoint(mountPoint), m_modes(modes), m_privilege(privilege), m_lock(0) {

}

FileStream::~FileStream() {
    Close();
}

int FileStream::Open() {
    spinlock_acquire(&m_lock);
    if (m_open) {
        spinlock_release(&m_lock);
        return ESUCCESS;
    }
    if (m_mountPoint == nullptr) {
        spinlock_release(&m_lock);
        return -ENODEV;
    }
    switch (m_mountPoint->type) {
        case FileSystemType::TMPFS:
            {
                using namespace TempFS;
                TempFSInode* inode = (TempFSInode*)m_inode;
                if (inode == nullptr || inode->GetType() != InodeType::File) {
                    spinlock_release(&m_lock);
                    return -EISDIR;
                }
                inode->Lock();
                inode->SetCurrentHead({0, false, nullptr, 0, 0});
                int rc = inode->Open();
                if (rc < 0) {
                    inode->Unlock();
                    spinlock_release(&m_lock);
                    return -ENOSYS;
                }
                TempFSInode::Head* head = new TempFSInode::Head(inode->GetCurrentHead());
                inode->Unlock();
                if (head == nullptr) {
                    spinlock_release(&m_lock);
                    return -ENOMEM;
                }
                m_inode_state = head;
            }
            break;
        default:
            spinlock_release(&m_lock);
            return -ENODEV;
            break;
    }
    m_open = true;
    spinlock_release(&m_lock);
    return ESUCCESS;
}

int FileStream::Close() {
    spinlock_acquire(&m_lock);
    if (!m_open) {
        spinlock_release(&m_lock);
        return ESUCCESS;
    }
    if (m_mountPoint == nullptr) {
        spinlock_release(&m_lock);
        return -ENODEV;
    }
    switch (m_mountPoint->type) {
        case FileSystemType::TMPFS:
            {
                using namespace TempFS;
                TempFSInode* inode = (TempFSInode*)m_inode;
                if (inode == nullptr || inode->GetType() != InodeType::File) {
                    spinlock_release(&m_lock);
                    return -EISDIR;
                }
                inode->Lock();
                inode->SetCurrentHead(*(TempFSInode::Head*)m_inode_state);
                m_open = false;
                int rc = inode->Close();
                inode->Unlock();
                delete (TempFSInode::Head*)m_inode_state;
                spinlock_release(&m_lock);
                return rc;
            }
            break;
        default:
            spinlock_release(&m_lock);
            return -ENODEV;
            break;
    }
    spinlock_release(&m_lock);
    return -ENOSYS;
}

int64_t FileStream::ReadStream(uint8_t* bytes, int64_t count, int* status) {
    if (count < 0)
        return -EINVAL;

    spinlock_acquire(&m_lock);
    if (m_mountPoint == nullptr) {
        spinlock_release(&m_lock);
        return -ENODEV;
    }

    if (!(m_modes & VFS_READ)) {
        spinlock_release(&m_lock);
        return -EACCES;
    }
    switch (m_mountPoint->type) {
        case FileSystemType::TMPFS:
            {
                using namespace TempFS;
                TempFSInode* inode = (TempFSInode*)m_inode;
                if (inode == nullptr || inode->GetType() != InodeType::File) {
                    spinlock_release(&m_lock);
                    return -EISDIR;
                }
                inode->Lock();
                inode->SetCurrentHead(*(TempFSInode::Head*)m_inode_state);
                int i_status;
                int64_t rc = inode->ReadStream(m_privilege, bytes, count, &i_status);
                if (status != nullptr)
                    *status = i_status;
                *(TempFSInode::Head*)m_inode_state = inode->GetCurrentHead();
                inode->Unlock();
                spinlock_release(&m_lock);
                return rc;
            }
            break;
        default:
            spinlock_release(&m_lock);
            return -ENODEV;
            break;
    }
    spinlock_release(&m_lock);
    return -ENOSYS;
}

int64_t FileStream::WriteStream(const uint8_t* bytes, int64_t count, int* status) {
    if (count < 0)
        return -EINVAL;
    spinlock_acquire(&m_lock);
    if (m_mountPoint == nullptr) {
        spinlock_release(&m_lock);
        return -ENODEV;
    }
    if (!(m_modes & VFS_WRITE)) {
        spinlock_release(&m_lock);
        return -EACCES;
    }
    switch (m_mountPoint->type) {
        case FileSystemType::TMPFS:
            {
                using namespace TempFS;
                TempFSInode* inode = (TempFSInode*)m_inode;
                if (inode == nullptr || inode->GetType() != InodeType::File) {
                    spinlock_release(&m_lock);
                    return -EISDIR;
                }
                inode->Lock();
                inode->SetCurrentHead(*(TempFSInode::Head*)m_inode_state);
                int i_status;
                int64_t rc = inode->WriteStream(m_privilege, bytes, count, &i_status);
                *(TempFSInode::Head*)m_inode_state = inode->GetCurrentHead();
                inode->Unlock();
                if (rc >= 0 && status != nullptr)
                    *status = i_status;
                spinlock_release(&m_lock);
                return rc;
            }
            break;
        default:
            spinlock_release(&m_lock);
            return -ENODEV;
            break;
    }
    spinlock_release(&m_lock);
    return -ENOSYS;
}

int FileStream::Seek(int64_t offset) {
    if (offset < 0)
        return -EINVAL;
    spinlock_acquire(&m_lock);
    if (m_mountPoint == nullptr) {
        spinlock_release(&m_lock);
        return -ENODEV;
    }
    switch (m_mountPoint->type) {
        case FileSystemType::TMPFS:
            {
                using namespace TempFS;
                TempFSInode* inode = (TempFSInode*)m_inode;
                if (inode == nullptr || inode->GetType() != InodeType::File) {
                    spinlock_release(&m_lock);
                    return -EISDIR;
                }
                inode->Lock();
                inode->SetCurrentHead(*(TempFSInode::Head*)m_inode_state);
                int rc = inode->Seek(offset);
                *(TempFSInode::Head*)m_inode_state = inode->GetCurrentHead();
                inode->Unlock();
                if (rc < 0) {
                    spinlock_release(&m_lock);
                    return rc;
                }
            }
            break;
        default:
            spinlock_release(&m_lock);
            return -ENODEV;
            break;
    }
    spinlock_release(&m_lock);
    return ESUCCESS;
}

int FileStream::Rewind() {
    spinlock_acquire(&m_lock);
    if (m_mountPoint == nullptr) {
        spinlock_release(&m_lock);
        return -ENODEV;
    }
    switch (m_mountPoint->type) {
        case FileSystemType::TMPFS:
            {
                using namespace TempFS;
                TempFSInode* inode = (TempFSInode*)m_inode;
                if (inode == nullptr || inode->GetType() != InodeType::File) {
                    spinlock_release(&m_lock);
                    return -EISDIR;
                }
                inode->Lock();
                inode->SetCurrentHead(*(TempFSInode::Head*)m_inode_state);
                int rc = inode->Rewind();
                *(TempFSInode::Head*)m_inode_state = inode->GetCurrentHead();
                inode->Unlock();
                if (rc < 0) {
                    spinlock_release(&m_lock);
                    return rc;
                }
            }
            break;
        default:
            spinlock_release(&m_lock);
            return -ENODEV;
            break;
    }
    spinlock_release(&m_lock);
    return ESUCCESS;
}

int64_t FileStream::GetOffset() const {
    spinlock_acquire(&m_lock);
    if (m_mountPoint == nullptr) {
        spinlock_release(&m_lock);
        return -ENODEV;
    }
    if (m_inode == nullptr || m_inode->GetType() != InodeType::File) {
        spinlock_release(&m_lock);
        return -EISDIR;
    }
    int64_t offset;
    switch (m_mountPoint->type) {
        case FileSystemType::TMPFS: {
            using namespace TempFS;
            TempFSInode* inode = (TempFSInode*)m_inode;
            inode->Lock();
            inode->SetCurrentHead(*(TempFSInode::Head*)m_inode_state);
            offset = inode->GetOffset();
            inode->Unlock();
            break;
        }
        default:
            spinlock_release(&m_lock);
            return -ENODEV;
            break;
    }
    spinlock_release(&m_lock);
    return offset;
}

bool FileStream::isOpen(int* status) const {
    spinlock_acquire(&m_lock);
    if (m_mountPoint == nullptr) {
        spinlock_release(&m_lock);
        if (status != nullptr)
            *status = -ENODEV;
        return false;
    }
    if (m_inode == nullptr || m_inode->GetType() != InodeType::File) {
        spinlock_release(&m_lock);
        if (status != nullptr)
            *status = -EISDIR;
        return false;
    }
    bool open;
    switch (m_mountPoint->type) {
        case FileSystemType::TMPFS: {
            using namespace TempFS;
            TempFSInode* inode = (TempFSInode*)m_inode;
            inode->Lock();
            inode->SetCurrentHead(*(TempFSInode::Head*)m_inode_state);
            open = inode->isOpen();
            inode->Unlock();
            break;
        }
        default:
            spinlock_release(&m_lock);
            if (status != nullptr)
                *status = -ENODEV;
            return false;
            break;
    }
    spinlock_release(&m_lock);
    if (status != nullptr)
        *status = ESUCCESS;
    return open;
}

size_t FileStream::GetSize(int* status) const {
    spinlock_acquire(&m_lock);
    if (m_inode == nullptr || m_inode->GetType() != InodeType::File) {
        spinlock_release(&m_lock);
        if (status != nullptr)
            *status = -EISDIR;
        return 0;
    }
    switch (m_mountPoint->type) {
    case FileSystemType::TMPFS: {
        using namespace TempFS;
        TempFSInode* inode = (TempFSInode*)m_inode;
        int i_status;
        inode->Lock();
        size_t size = inode->GetSize(&i_status);
        inode->Unlock();
        spinlock_release(&m_lock);
        if (status != nullptr)
            *status = i_status;
        return size;
    }
    default:
        spinlock_release(&m_lock);
        if (status != nullptr)
            *status = -ENODEV;
        return 0;
    }
    spinlock_release(&m_lock);
    if (status != nullptr)
        *status = -ENOSYS;
    return 0;
}

Inode* FileStream::GetInode() const {
    return m_inode;
}

FileSystem* FileStream::GetFileSystem(int* status) const {
    spinlock_acquire(&m_lock);
    if (m_mountPoint == nullptr) {
        spinlock_release(&m_lock);
        if (status != nullptr)
            *status = -ENODEV;
        return nullptr;
    }
    if (status != nullptr)
        *status = ESUCCESS;
    return m_mountPoint->fs;
}

VFS_MountPoint* FileStream::GetMountPoint() const {
    return m_mountPoint;
}
