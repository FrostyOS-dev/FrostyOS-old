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

#ifndef _VFS_HPP
#define _VFS_HPP

#include <stdint.h>
#include <stddef.h>

#include "DirectoryStream.hpp"
#include "FileSystem.hpp"
#include "Inode.hpp"

#include <Data-structures/LinkedList.hpp>


constexpr uint8_t VFS_READ = 1;
constexpr uint8_t VFS_WRITE = 2;


struct VFS_MountPoint {
    FileSystemType type;
    FileSystem* fs;
    Inode* RootInode;
    VFS_MountPoint* parent;
};

struct VFS_WorkingDirectory {
    VFS_MountPoint* mountpoint;
    Inode* inode;
};

class FileStream; // defined in FileStream.hpp

class VFS final : public FileSystem {
public:
    VFS();
    VFS(FileSystemType root_type);
    ~VFS();

    int MountRoot(FileSystemType type);
    int Mount(FilePrivilegeLevel current_privilege, const char* path, FileSystemType type);
    int Unmount(FilePrivilegeLevel current_privilege, const char* path, VFS_WorkingDirectory* working_directory = nullptr);

    int CreateFile(FilePrivilegeLevel current_privilege, const char* parent, const char* name, size_t size = 0, bool inherit_permissions = true, FilePrivilegeLevel privilege = {0, 0, 00644}) override;
    int CreateFolder(FilePrivilegeLevel current_privilege, const char* parent, const char* name, bool inherit_permissions = true, FilePrivilegeLevel privilege = {0, 0, 00644}) override;
    int CreateSymLink(FilePrivilegeLevel current_privilege, const char* parent, const char* name, const char* target, bool inherit_permissions = true, FilePrivilegeLevel privilege = {0, 0, 00644}) override;

    int DeleteInode(FilePrivilegeLevel current_privilege, const char* path, bool recursive = false, bool delete_name = false) override; // delete_name is not used by the VFS as it **always** deletes the name

    FileStream* OpenStream(FilePrivilegeLevel current_privilege, const char* path, uint8_t modes, VFS_WorkingDirectory* working_directory = nullptr, int* status = nullptr); // status will be set if not nullptr.
    int CloseStream(FileStream* stream);

    DirectoryStream* OpenDirectoryStream(FilePrivilegeLevel current_privilege, const char* path, uint8_t modes, VFS_WorkingDirectory* working_directory = nullptr, int* status = nullptr); // status will be set if not nullptr.
    int CloseDirectoryStream(DirectoryStream* stream);

    bool IsValidPath(const char* path, VFS_WorkingDirectory* working_directory = nullptr, int* status = nullptr) const; // status will be set if not nullptr.

    Inode* GetInode(const char* path, VFS_WorkingDirectory* working_directory = nullptr, FileSystem** fs = nullptr, VFS_MountPoint** mountpoint = nullptr, int* status = nullptr) const; // status will be set if not nullptr.

    VFS_MountPoint* GetMountPoint(FileSystem* fs, int* status = nullptr) const; // status will be set if not nullptr.

    VFS_WorkingDirectory* GetRootWorkingDirectory() const;

    int DestroyFileSystem() override; // does not currently do anything.

    // These to functions are just parsed to the root mountpoint.
    Inode* GetRootInode(uint64_t index, int* status = nullptr) const override; // status will be set if not nullptr
    uint64_t GetRootInodeCount() const override;

    FileSystemType GetType() const override;

    FilePrivilegeLevel GetRootPrivilege() const override;

private:
    VFS_MountPoint* GetMountPoint(const char* path, VFS_WorkingDirectory* working_directory = nullptr, Inode** inode = nullptr, int* status = nullptr) const; // status will be set if not nullptr.

    // Get the mountpoint for a child of the given path.
    VFS_MountPoint* GetChildMountPoint(const char* path, VFS_WorkingDirectory* working_directory = nullptr, Inode** inode = nullptr, int* status = nullptr) const; // status will be set if not nullptr.

    bool isMountpoint(const char* path, size_t len, VFS_WorkingDirectory* working_directory = nullptr, int* status = nullptr); // When false is returned, the caller **MUST** check for any errors. status will be set if not nullptr.

private:
    VFS_MountPoint* m_root;

    LinkedList::LockableLinkedList<VFS_MountPoint> m_mountPoints;
    LinkedList::LockableLinkedList<FileStream> m_streams;
    LinkedList::LockableLinkedList<DirectoryStream> m_directoryStreams;
};

extern VFS* g_VFS;

#endif /* _VFS_HPP */