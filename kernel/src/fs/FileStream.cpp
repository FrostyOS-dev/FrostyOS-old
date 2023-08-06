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

FileStream::FileStream(Inode* inode, VFS_MountPoint* mountPoint, uint8_t modes) : m_inode(inode), m_mountPoint(mountPoint), m_modes(modes) {

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

bool FileStream::ReadStream(uint8_t* bytes, uint64_t count) {
    if (m_mountPoint == nullptr) {
        SetLastError(FileStreamError::INVALID_MOUNTPOINT);
        return false;
    }

    if (!(m_modes & VFS_READ)) {
        SetLastError(FileStreamError::INVALID_MODE);
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
                if (!inode->ReadStream(bytes, count)) {
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

bool FileStream::WriteStream(const uint8_t* bytes, uint64_t count) {
    if (m_mountPoint == nullptr) {
        SetLastError(FileStreamError::INVALID_MOUNTPOINT);
        return false;
    }
    if (!(m_modes & VFS_WRITE)) {
        SetLastError(FileStreamError::INVALID_MODE);
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
                if (!inode->WriteStream(bytes, count)) {
                    InodeError error = inode->GetLastError();
                    if (error == InodeError::INVALID_ARGUMENTS)
                        SetLastError(FileStreamError::INVALID_ARGUMENTS);
                    else if (error == InodeError::ALLOCATION_FAILED)
                        SetLastError(FileStreamError::ALLOCATION_FAILED);
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
        return 0;
    }
    SetLastError(FileStreamError::SUCCESS);
    return m_inode->isOpen();
}

FileStreamError FileStream::GetLastError() const {
    return m_lastError;
}

/* Private members */

void FileStream::SetLastError(FileStreamError error) const {
    m_lastError = error;
}
