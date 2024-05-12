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

#ifndef _FILE_STREAM_HPP
#define _FILE_STREAM_HPP

#include "VFS.hpp"
#include "Inode.hpp"
#include "FilePrivilegeLevel.hpp"

#include <stdint.h>
#include <spinlock.h>

class FileStream {
public:
    FileStream(Inode* inode, VFS_MountPoint* mountPoint, uint8_t modes, FilePrivilegeLevel privilege);
    ~FileStream();

    int Open();
    int Close();
    int64_t ReadStream(uint8_t* bytes, int64_t count = 1, int* status = nullptr); // status will be set if not nullptr
    int64_t WriteStream(const uint8_t* bytes, int64_t count = 1, int* status = nullptr); // status will be set if not nullptr
    int Seek(int64_t offset);
    int Rewind();
    int64_t GetOffset() const;
    bool isOpen(int* status = nullptr) const; // status will be set if not nullptr
    size_t GetSize(int* status = nullptr) const; // status will be set if not nullptr

    Inode* GetInode() const;
    FileSystem* GetFileSystem(int* status = nullptr) const; // status will be set if not nullptr

    VFS_MountPoint* GetMountPoint() const;

private:
    bool m_open;
    Inode* m_inode;
    void* m_inode_state;
    VFS_MountPoint* m_mountPoint;
    uint8_t m_modes;
    FilePrivilegeLevel m_privilege;

    mutable spinlock_t m_lock;
};

#endif /* _FILE_STREAM_HPP */