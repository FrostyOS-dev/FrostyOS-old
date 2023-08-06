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

class VFS : public FileSystem {
public:
    VFS();
    VFS(FileSystemType root_type);
    ~VFS();

    bool MountRoot(FileSystemType type);
    bool Mount(const char* path, FileSystemType type);

    bool CreateFile(const char* parent, const char* name, size_t size = 0) override;
    bool CreateFolder(const char* parent, const char* name) override;

    bool DeleteInode(const char* path, bool recursive = false) override;

    FileStream* OpenStream(const char* path, uint8_t modes);
    bool CloseStream(FileStream* stream);

private:
    VFS_MountPoint* GetMountPoint(const char* path, Inode** inode = nullptr);

    bool isMountpoint(const char* path, size_t len); // When false is returned, the caller **MUST** check for any errors.

private:
    VFS_MountPoint* m_root;

    LinkedList::SimpleLinkedList<VFS_MountPoint> m_mountPoints;
    LinkedList::SimpleLinkedList<FileStream> m_streams;
};

extern VFS* g_VFS;

#endif /* _VFS_HPP */