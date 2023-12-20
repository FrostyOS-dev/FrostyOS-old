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

#include "DirectoryStream.hpp"

DirectoryStream::DirectoryStream(Inode* inode, FilePrivilegeLevel privilege, FileSystemType fs_type, FileSystem* fs) : m_inode(inode), m_privilege(privilege), m_fs_type(fs_type), m_fs(fs), m_index(0), m_isOpen(false), m_last_error(DirectoryStreamError::SUCCESS) {

}

DirectoryStream::~DirectoryStream() {

}

bool DirectoryStream::Open() {
    if (m_isOpen) {
        SetLastError(DirectoryStreamError::SUCCESS); // already open
        return true;
    }
    if (m_inode == nullptr && m_fs == nullptr) { // nothing to fetch child inodes from
        SetLastError(DirectoryStreamError::INVALID_INODE);
        return false;
    }
    if (m_inode != nullptr && m_inode->GetType() != InodeType::Folder) {
        SetLastError(DirectoryStreamError::INVALID_INODE);
        return false;
    }
    FilePrivilegeLevel privilege;
    if (m_inode != nullptr)
        privilege = m_inode->GetPrivilegeLevel();
    else
        privilege = m_fs->GetRootPrivilege();
    if (privilege.UID == m_privilege.UID || privilege.UID == 0) {
        if (!((privilege.ACL & ACL_USER_READ) > 0)) {
            SetLastError(DirectoryStreamError::NO_PERMISSION);
            return false;
        }
    }
    else if (privilege.GID == m_privilege.GID) {
        if (!((privilege.ACL & ACL_GROUP_READ) > 0)) {
            SetLastError(DirectoryStreamError::NO_PERMISSION);
            return false;
        }
    }
    else {
        if (!((privilege.ACL & ACL_OTHER_READ) > 0)) {
            SetLastError(DirectoryStreamError::NO_PERMISSION);
            return false;
        }
    }
    m_index = 0;
    m_isOpen = true;
    SetLastError(DirectoryStreamError::SUCCESS);
    return true;
}

bool DirectoryStream::Close() {
    m_isOpen = false;
    SetLastError(DirectoryStreamError::SUCCESS);
    return true;
}

bool DirectoryStream::Seek(uint64_t index) {
    if (!m_isOpen) {
        SetLastError(DirectoryStreamError::STREAM_CLOSED);
        return false;
    }
    if (index >= m_inode->GetChildCount()) {
        SetLastError(DirectoryStreamError::INVALID_ARGUMENTS);
        return false;
    }
    m_index = index;
    SetLastError(DirectoryStreamError::SUCCESS);
    return true;
}

uint64_t DirectoryStream::GetOffset() const {
    return m_index;
}

Inode* DirectoryStream::GetNextInode() {
    if (!m_isOpen) {
        SetLastError(DirectoryStreamError::STREAM_CLOSED);
        return nullptr;
    }
    Inode* inode = nullptr;
    if (m_inode != nullptr) {
        if (m_index >= m_inode->GetChildCount()) {
            SetLastError(DirectoryStreamError::INVALID_ARGUMENTS);
            return nullptr;
        }
        inode = m_inode->GetChild(m_index);
    }
    else if (m_fs != nullptr) {
        if (m_index >= m_fs->GetRootInodeCount()) {
            SetLastError(DirectoryStreamError::INVALID_ARGUMENTS);
            return nullptr;
        }
        inode = m_fs->GetRootInode(m_index);
    }
    else {
        SetLastError(DirectoryStreamError::INVALID_INODE);
        return nullptr;
    }
    m_index++;
    SetLastError(DirectoryStreamError::SUCCESS);
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

DirectoryStreamError DirectoryStream::GetLastError() const {
    return m_last_error;
}

void DirectoryStream::SetLastError(DirectoryStreamError error) {
    m_last_error = error;
}