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

#include "FileStream.hpp"

#include "TempFS/TempFSInode.hpp"

FileStream::FileStream(Inode* inode, VFS_MountPoint* mountPoint, uint8_t modes, FilePrivilegeLevel privilege) : m_inode(inode), m_mountPoint(mountPoint), m_modes(modes), m_privilege(privilege) {

}

FileStream::~FileStream() {
    Close();
}

bool FileStream::Open() {
    if (m_mountPoint == nullptr) {
        SetLastError(FileStreamError::INVALID_MOUNTPOINT);
        return false;
    }
    switch (m_mountPoint->type) {
        case FileSystemType::TMPFS:
            {
                using namespace TempFS;
                TempFSInode* inode = (TempFSInode*)m_inode;
                if (inode == nullptr || inode->GetType() != InodeType::File) {
                    SetLastError(FileStreamError::INVALID_INODE);
                    return false;
                }
                if (!inode->Open()) {
                    SetLastError(FileStreamError::INTERNAL_ERROR);
                    return false;
                }
            }
            break;
        default:
            SetLastError(FileStreamError::INVALID_FS_TYPE);
            return false;
            break;
    }
    SetLastError(FileStreamError::SUCCESS);
    return true;
}

bool FileStream::Close() {
    if (m_mountPoint == nullptr) {
        SetLastError(FileStreamError::INVALID_MOUNTPOINT);
        return false;
    }
    switch (m_mountPoint->type) {
        case FileSystemType::TMPFS:
            {
                using namespace TempFS;
                TempFSInode* inode = (TempFSInode*)m_inode;
                if (inode == nullptr || inode->GetType() != InodeType::File) {
                    SetLastError(FileStreamError::INVALID_INODE);
                    return false;
                }
                if (!inode->Close()) {
                    SetLastError(FileStreamError::INTERNAL_ERROR);
                    return false;
                }
            }
            break;
        default:
            SetLastError(FileStreamError::INVALID_FS_TYPE);
            return false;
            break;
    }
    SetLastError(FileStreamError::SUCCESS);
    return true;
}

uint64_t FileStream::ReadStream(uint8_t* bytes, uint64_t count) {
    if (m_mountPoint == nullptr) {
        SetLastError(FileStreamError::INVALID_MOUNTPOINT);
        return 0;
    }

    if (!(m_modes & VFS_READ)) {
        SetLastError(FileStreamError::INVALID_MODE);
        return 0;
    }
    switch (m_mountPoint->type) {
        case FileSystemType::TMPFS:
            {
                using namespace TempFS;
                TempFSInode* inode = (TempFSInode*)m_inode;
                if (inode == nullptr || inode->GetType() != InodeType::File) {
                    SetLastError(FileStreamError::INVALID_INODE);
                    return 0;
                }
                uint64_t status = inode->ReadStream(m_privilege, bytes, count);
                if (status != count) {
                    InodeError error = inode->GetLastError();
                    if (error == InodeError::INVALID_ARGUMENTS)
                        SetLastError(FileStreamError::INVALID_ARGUMENTS);
                    else if (error == InodeError::STREAM_CLOSED)
                        SetLastError(FileStreamError::STREAM_CLOSED);
                    else if (error == InodeError::NO_PERMISSION)
                        SetLastError(FileStreamError::NO_PERMISSION);
                    else
                        SetLastError(FileStreamError::INTERNAL_ERROR);
                }
                else
                    SetLastError(FileStreamError::SUCCESS);
                return status;
            }
            break;
        default:
            SetLastError(FileStreamError::INVALID_FS_TYPE);
            return 0;
            break;
    }
    SetLastError(FileStreamError::INTERNAL_ERROR); // should be unreachable
    return 0;
}

uint64_t FileStream::WriteStream(const uint8_t* bytes, uint64_t count) {
    if (m_mountPoint == nullptr) {
        SetLastError(FileStreamError::INVALID_MOUNTPOINT);
        return 0;
    }
    if (!(m_modes & VFS_WRITE)) {
        SetLastError(FileStreamError::INVALID_MODE);
        return 0;
    }
    switch (m_mountPoint->type) {
        case FileSystemType::TMPFS:
            {
                using namespace TempFS;
                TempFSInode* inode = (TempFSInode*)m_inode;
                if (inode == nullptr || inode->GetType() != InodeType::File) {
                    SetLastError(FileStreamError::INVALID_INODE);
                    return 0;
                }
                uint64_t status = inode->WriteStream(m_privilege, bytes, count);
                if (status != count) {
                    InodeError error = inode->GetLastError();
                    if (error == InodeError::INVALID_ARGUMENTS)
                        SetLastError(FileStreamError::INVALID_ARGUMENTS);
                    else if (error == InodeError::ALLOCATION_FAILED)
                        SetLastError(FileStreamError::ALLOCATION_FAILED);
                    else if (error == InodeError::STREAM_CLOSED)
                        SetLastError(FileStreamError::STREAM_CLOSED);
                    else if (error == InodeError::NO_PERMISSION)
                        SetLastError(FileStreamError::NO_PERMISSION);
                    else
                        SetLastError(FileStreamError::INTERNAL_ERROR);
                }
                else
                    SetLastError(FileStreamError::SUCCESS);
                return status;
            }
            break;
        default:
            SetLastError(FileStreamError::INVALID_FS_TYPE);
            return 0;
            break;
    }
    SetLastError(FileStreamError::INTERNAL_ERROR); // should be unreachable
    return 0;
}

bool FileStream::Seek(uint64_t offset) {
    if (m_mountPoint == nullptr) {
        SetLastError(FileStreamError::INVALID_MOUNTPOINT);
        return false;
    }
    switch (m_mountPoint->type) {
        case FileSystemType::TMPFS:
            {
                using namespace TempFS;
                TempFSInode* inode = (TempFSInode*)m_inode;
                if (inode == nullptr || inode->GetType() != InodeType::File) {
                    SetLastError(FileStreamError::INVALID_INODE);
                    return false;
                }
                if (!inode->Seek(offset)) {
                    InodeError error = inode->GetLastError();
                    if (error == InodeError::INVALID_ARGUMENTS)
                        SetLastError(FileStreamError::INVALID_ARGUMENTS);
                    else if (error == InodeError::STREAM_CLOSED)
                        SetLastError(FileStreamError::STREAM_CLOSED);
                    else
                        SetLastError(FileStreamError::INTERNAL_ERROR);
                    return false;
                }
            }
            break;
        default:
            SetLastError(FileStreamError::INVALID_FS_TYPE);
            return false;
            break;
    }
    SetLastError(FileStreamError::SUCCESS);
    return true;
}

bool FileStream::Rewind() {
    if (m_mountPoint == nullptr) {
        SetLastError(FileStreamError::INVALID_MOUNTPOINT);
        return false;
    }
    switch (m_mountPoint->type) {
        case FileSystemType::TMPFS:
            {
                using namespace TempFS;
                TempFSInode* inode = (TempFSInode*)m_inode;
                if (inode == nullptr || inode->GetType() != InodeType::File) {
                    SetLastError(FileStreamError::INVALID_INODE);
                    return false;
                }
                if (!inode->Rewind()) {
                    if (inode->GetLastError() == InodeError::STREAM_CLOSED)
                        SetLastError(FileStreamError::STREAM_CLOSED);
                    else
                        SetLastError(FileStreamError::INTERNAL_ERROR);
                    return false;
                }
            }
            break;
        default:
            SetLastError(FileStreamError::INVALID_FS_TYPE);
            return false;
            break;
    }
    SetLastError(FileStreamError::SUCCESS);
    return true;
}

uint64_t FileStream::GetOffset() const {
    if (m_mountPoint == nullptr) {
        SetLastError(FileStreamError::INVALID_MOUNTPOINT);
        return false;
    }
    if (m_inode == nullptr || m_inode->GetType() != InodeType::File) {
        SetLastError(FileStreamError::INVALID_INODE);
        return 0;
    }
    SetLastError(FileStreamError::SUCCESS);
    return m_inode->GetOffset();
}

bool FileStream::isOpen() const {
    if (m_mountPoint == nullptr) {
        SetLastError(FileStreamError::INVALID_MOUNTPOINT);
        return false;
    }
    if (m_inode == nullptr || m_inode->GetType() != InodeType::File) {
        SetLastError(FileStreamError::INVALID_INODE);
        return false;
    }
    SetLastError(FileStreamError::SUCCESS);
    return m_inode->isOpen();
}

size_t FileStream::GetSize() const {
    if (m_inode == nullptr || m_inode->GetType() != InodeType::File) {
        SetLastError(FileStreamError::INVALID_INODE);
        return 0;
    }
    switch (m_mountPoint->type) {
    case FileSystemType::TMPFS:
        return ((TempFS::TempFSInode*)m_inode)->GetSize();
    default:
        SetLastError(FileStreamError::INVALID_FS_TYPE);
        return 0;
    }
    SetLastError(FileStreamError::INTERNAL_ERROR); // should be unreachable
    return 0;
}

Inode* FileStream::GetInode() const {
    SetLastError(FileStreamError::SUCCESS);
    return m_inode;
}

FileSystem* FileStream::GetFileSystem() const {
    if (m_mountPoint == nullptr) {
        SetLastError(FileStreamError::INVALID_MOUNTPOINT);
        return nullptr;
    }
    SetLastError(FileStreamError::SUCCESS);
    return m_mountPoint->fs;
}

FileStreamError FileStream::GetLastError() const {
    return m_lastError;
}

/* Private members */

void FileStream::SetLastError(FileStreamError error) const {
    m_lastError = error;
}
