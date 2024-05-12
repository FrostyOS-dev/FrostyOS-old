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

#ifndef _FILE_SYSTEM_HPP
#define _FILE_SYSTEM_HPP

#include <stdint.h>
#include <stddef.h>

#include "FilePrivilegeLevel.hpp"
#include "Inode.hpp"

enum class FileSystemType {
    VFS = 0,
    TMPFS = 1
};

constexpr char PATH_SEPARATOR = '/';

class FileSystem {
public:
    virtual ~FileSystem() {}

    virtual int CreateFile(FilePrivilegeLevel current_privilege, const char* parent, const char* name, size_t size = 0, bool inherit_permissions = true, FilePrivilegeLevel privilege = {0, 0, 00644}) = 0;
    virtual int CreateFolder(FilePrivilegeLevel current_privilege, const char* parent, const char* name, bool inherit_permissions = true, FilePrivilegeLevel privilege = {0, 0, 00644}) = 0;
    virtual int CreateSymLink(FilePrivilegeLevel current_privilege, const char* parent, const char* name, const char* target, bool inherit_permissions = true, FilePrivilegeLevel privilege = {0, 0, 00644}) = 0;

    virtual int DeleteInode(FilePrivilegeLevel current_privilege, const char* path, bool recursive = false, bool delete_name = false) = 0;

    virtual int DestroyFileSystem() = 0;

    virtual Inode* GetRootInode(uint64_t index, int* status = nullptr) const = 0; // status will be set if not nullptr
    virtual uint64_t GetRootInodeCount() const = 0;

    virtual FileSystemType GetType() const = 0;

    virtual FilePrivilegeLevel GetRootPrivilege() const = 0;

protected:
    // Standard members
    size_t p_blockSize;
};

#endif /* _FILE_SYSTEM_HPP */