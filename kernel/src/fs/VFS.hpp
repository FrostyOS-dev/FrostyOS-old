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

#ifndef _VFS_HPP
#define _VFS_HPP

#include <stdint.h>
#include <stddef.h>

#include "FileSystem.hpp"
#include "Inode.hpp"

#include <Data-structures/LinkedList.hpp>


constexpr uint8_t VFS_READ = 1;
constexpr uint8_t VFS_WRITE = 2;


struct VFS_MountPoint {
    FileSystemType type;
    FileSystem* fs;
    Inode* RootInode;
};

class FileStream; // defined in FileStream.hpp

class VFS final : public FileSystem {
public:
    VFS();
    VFS(FileSystemType root_type);
    ~VFS();

    bool MountRoot(FileSystemType type);
    bool Mount(const char* path, FileSystemType type);

    bool CreateFile(FilePrivilegeLevel current_privilege, const char* parent, const char* name, size_t size = 0, bool inherit_permissions = true, FilePrivilegeLevel privilege = {0, 0, 00644}) override;
    bool CreateFolder(FilePrivilegeLevel current_privilege, const char* parent, const char* name, bool inherit_permissions = true, FilePrivilegeLevel privilege = {0, 0, 00644}) override;
    bool CreateSymLink(FilePrivilegeLevel current_privilege, const char* parent, const char* name, const char* target, bool inherit_permissions = true, FilePrivilegeLevel privilege = {0, 0, 00644}) override;

    bool DeleteInode(FilePrivilegeLevel current_privilege, const char* path, bool recursive = false) override;

    FileStream* OpenStream(FilePrivilegeLevel current_privilege, const char* path, uint8_t modes);
    bool CloseStream(FileStream* stream);

    bool IsValidPath(const char* path) const;

    Inode* GetInode(const char* path, FileSystem** fs = nullptr) const;

private:
    VFS_MountPoint* GetMountPoint(const char* path, Inode** inode = nullptr) const;

    bool isMountpoint(const char* path, size_t len); // When false is returned, the caller **MUST** check for any errors.

private:
    VFS_MountPoint* m_root;

    LinkedList::SimpleLinkedList<VFS_MountPoint> m_mountPoints;
    LinkedList::SimpleLinkedList<FileStream> m_streams;
};

extern VFS* g_VFS;

#endif /* _VFS_HPP */