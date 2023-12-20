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

#ifndef _DIRECTORY_STREAM_HPP
#define _DIRECTORY_STREAM_HPP

#include <stdint.h>

#include "Inode.hpp"
#include "FilePrivilegeLevel.hpp"
#include "FileSystem.hpp"

enum class DirectoryStreamError {
    SUCCESS = 0,
    INVALID_ARGUMENTS = 1,
    STREAM_CLOSED = 2,
    INVALID_INODE = 3,
    NO_PERMISSION = 4,
    INTERNAL_ERROR = 5
};

class DirectoryStream {
public:
    DirectoryStream(Inode* inode, FilePrivilegeLevel privilege, FileSystemType fs_type, FileSystem* fs = nullptr); // fs must be parsed when inode is nullptr
    ~DirectoryStream();

    bool Open();
    bool Close();

    bool Seek(uint64_t index);

    uint64_t GetOffset() const;

    Inode* GetNextInode();

    bool IsOpen() const;

    Inode* GetInode() const;

    FileSystem* GetFileSystem() const;

    FileSystemType GetFileSystemType() const;

    DirectoryStreamError GetLastError() const;

private:

    void SetLastError(DirectoryStreamError error);

private:
    Inode* m_inode;
    FilePrivilegeLevel m_privilege;
    FileSystemType m_fs_type;
    FileSystem* m_fs;
    uint64_t m_index;
    bool m_isOpen;

    DirectoryStreamError m_last_error;
};

#endif /* _DIRECTORY_STREAM_HPP */