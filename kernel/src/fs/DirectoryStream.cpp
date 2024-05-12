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

#include "DirectoryStream.hpp"
#include "spinlock.h"
#include <errno.h>

DirectoryStream::DirectoryStream(Inode* inode, FilePrivilegeLevel privilege, FileSystemType fs_type, FileSystem* fs) : m_inode(inode), m_privilege(privilege), m_fs_type(fs_type), m_fs(fs), m_index(0), m_isOpen(false), m_lock(0) {

}

DirectoryStream::~DirectoryStream() {

}

int DirectoryStream::Open() {
    spinlock_acquire(&m_lock);
    if (m_isOpen) {
        spinlock_release(&m_lock);
        return ESUCCESS;
    }
    if (m_inode == nullptr && m_fs == nullptr) { // nothing to fetch child inodes from
        spinlock_release(&m_lock);
        return -ENOSYS;
    }
    if (m_inode != nullptr && m_inode->GetType() != InodeType::Folder) {
        spinlock_release(&m_lock);
        return -ENOTDIR;
    }
    FilePrivilegeLevel privilege;
    if (m_inode != nullptr)
        privilege = m_inode->GetPrivilegeLevel();
    else
        privilege = m_fs->GetRootPrivilege();
    if (privilege.UID == m_privilege.UID || privilege.UID == 0) {
        if (!((privilege.ACL & ACL_USER_READ) > 0)) {
            spinlock_release(&m_lock);
            return -EACCES;
        }
    }
    else if (privilege.GID == m_privilege.GID) {
        if (!((privilege.ACL & ACL_GROUP_READ) > 0)) {
            spinlock_release(&m_lock);
            return -EACCES;
        }
    }
    else {
        if (!((privilege.ACL & ACL_OTHER_READ) > 0)) {
            spinlock_release(&m_lock);
            return -EACCES;
        }
    }
    m_index = 0;
    m_isOpen = true;
    spinlock_release(&m_lock);
    return ESUCCESS;
}

int DirectoryStream::Close() {
    spinlock_acquire(&m_lock);
    m_isOpen = false;
    spinlock_release(&m_lock);
    return ESUCCESS;
}

int DirectoryStream::Seek(int64_t index) {
    if (index < 0)
        return -EINVAL;
    spinlock_acquire(&m_lock);
    if (!m_isOpen) {
        spinlock_release(&m_lock);
        return -EBADF;
    }
    if ((uint64_t)index >= m_inode->GetChildCount()) {
        spinlock_release(&m_lock);
        return -EINVAL;
    }
    m_index = index;
    return ESUCCESS;
}

int64_t DirectoryStream::GetOffset() const {
    return m_index;
}

Inode* DirectoryStream::GetNextInode(int* status) {
    spinlock_acquire(&m_lock);
    if (!m_isOpen) {
        spinlock_release(&m_lock);
        if (status != nullptr)
            *status = -EBADF;
        return nullptr;
    }
    Inode* inode = nullptr;
    if (m_inode != nullptr) {
        if ((uint64_t)m_index >= m_inode->GetChildCount()) {
            spinlock_release(&m_lock);
            if (status != nullptr)
                *status = -EINVAL;
            return nullptr;
        }
        inode = m_inode->GetChild(m_index);
    }
    else if (m_fs != nullptr) {
        if ((uint64_t)m_index >= m_fs->GetRootInodeCount()) {
            spinlock_release(&m_lock);
            if (status != nullptr)
                *status = -EINVAL;
            return nullptr;
        }
        inode = m_fs->GetRootInode(m_index);
    }
    else {
        spinlock_release(&m_lock);
        if (status != nullptr)
            *status = -ENOSYS;
        return nullptr;
    }
    m_index++;
    spinlock_release(&m_lock);
    if (status != nullptr)
        *status = ESUCCESS;
    return inode;
}

bool DirectoryStream::IsOpen() const {
    return m_isOpen;
}

Inode* DirectoryStream::GetInode() const {
    return m_inode;
}

FileSystem* DirectoryStream::GetFileSystem() const {
    return m_fs;
}

FileSystemType DirectoryStream::GetFileSystemType() const {
    return m_fs_type;
}
