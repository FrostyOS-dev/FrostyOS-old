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

#ifndef _TEMP_FILE_SYSTEM_HPP
#define _TEMP_FILE_SYSTEM_HPP

#include "../FileSystem.hpp"

#include "TempFSInode.hpp"

#include <Data-structures/LinkedList.hpp>

namespace TempFS {

    

    class TempFileSystem : public FileSystem {
    public:
        TempFileSystem(size_t blockSize);
        ~TempFileSystem();

        bool CreateFile(const char* parent, const char* name, size_t size = 0) override;
        bool CreateFile(TempFSInode* parent, const char* name, size_t size = 0);
        bool CreateFolder(const char* parent, const char* name) override;
        bool CreateFolder(TempFSInode* parent, const char* name);

        bool DeleteInode(const char* path, bool recursive = false) override;
        bool DeleteInode(TempFSInode* inode, bool recursive = false);

        void CreateNewRootInode(TempFSInode* inode);
        void DeleteRootInode(TempFSInode* inode);

        TempFSInode* GetInode(const char* path, TempFSInode** lastInode = nullptr, int64_t* end_index = nullptr); // last_inode and end_index are only filled if it is a non-null pointer

    private:
        LinkedList::SimpleLinkedList<TempFSInode> m_rootInodes;
    };
}

#endif /* _TEMP_FILE_SYSTEM_HPP */