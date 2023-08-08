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

#ifndef _TEMP_FS_INODE_HPP
#define _TEMP_FS_INODE_HPP

#include "../Inode.hpp"

#include <util.h>

#include <Data-structures/LinkedList.hpp>


namespace TempFS {
    class TempFileSystem;

    class TempFSInode : public Inode {
    public:
        TempFSInode();
        ~TempFSInode() override;

        bool Create(const char* name, TempFSInode* parent, InodeType type, TempFileSystem* fileSystem, size_t blockSize = PAGE_SIZE, uint32_t seed = 0);
        bool Delete();

        bool Open() override;
        bool Close() override;
        bool ReadStream(uint8_t* bytes, uint64_t count = 1) override;
        bool WriteStream(const uint8_t* bytes, uint64_t count = 1) override;
        bool Seek(uint64_t offset) override;
        bool Rewind() override;

        bool Read(uint64_t offset, uint8_t* bytes, uint64_t count = 1) override;
        bool Write(uint64_t offset, const uint8_t* bytes, uint64_t count = 1) override;

        bool Expand(size_t new_size) override;

        bool AddChild(TempFSInode* child);
        TempFSInode* GetChild(uint64_t ID) const;
        TempFSInode* GetChild(const char* name) const;
        uint64_t GetChildCount() const;
        bool RemoveChild(TempFSInode* child);

        bool SetParent(TempFSInode* parent);
        TempFSInode* GetParent() const override;

        void ResetID(uint32_t seed = 0) override;

    protected:

        void SetLastError(InodeError error) const override;

    private:
        TempFSInode* m_parent;
        TempFileSystem* m_fileSystem;
        uint32_t m_UID;
        uint32_t m_GID;
        uint16_t m_ACL; // only 12 lowest bits are used

        LinkedList::SimpleLinkedList<TempFSInode> m_children;

        /* Only for files */

        struct MemBlock {
            void* address;
            size_t size;
        };

        LinkedList::SimpleLinkedList<MemBlock> m_data;
        size_t m_size;
        MemBlock* m_currentBlock;
        uint64_t m_currentBlockIndex;
        size_t m_currentBlockOffset; // offset within a block
    };
}

#endif /* _TEMP_FS_INODE_HPP */