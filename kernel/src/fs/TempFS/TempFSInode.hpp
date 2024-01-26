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

#ifndef _TEMP_FS_INODE_HPP
#define _TEMP_FS_INODE_HPP

#include "../Inode.hpp"

#include <util.h>

#include <Data-structures/LinkedList.hpp>


namespace TempFS {
    class TempFileSystem;

    class TempFSInode : public Inode {
    public:
        struct Head {
            uint64_t CurrentOffset;
            bool isOpen;
            
            void* currentBlock;
            uint64_t currentBlockIndex;
            size_t currentBlockOffset;
        };

        TempFSInode();
        ~TempFSInode() override;

        bool Create(const char* name, TempFSInode* parent, InodeType type, TempFileSystem* fileSystem, FilePrivilegeLevel privilege, size_t blockSize = PAGE_SIZE, void* extra = nullptr, uint32_t seed = 0);
        bool Delete(bool delete_target = false, bool delete_name = false); // normally just deletes this and not the target

        bool Open() override;
        bool Close() override;
        uint64_t ReadStream(FilePrivilegeLevel privilege, uint8_t* bytes, uint64_t count = 1) override;
        uint64_t WriteStream(FilePrivilegeLevel privilege, const uint8_t* bytes, uint64_t count = 1) override;
        bool Seek(uint64_t offset) override;
        bool Rewind() override;

        uint64_t Read(FilePrivilegeLevel privilege, uint64_t offset, uint8_t* bytes, uint64_t count = 1) override;
        uint64_t Write(FilePrivilegeLevel privilege, uint64_t offset, const uint8_t* bytes, uint64_t count = 1) override;

        bool Expand(size_t new_size) override;

        InodeType GetType() const override;
        void SetType(InodeType type) override;

        bool AddChild(TempFSInode* child);
        TempFSInode* GetTMPFSChild(uint64_t ID) const;
        TempFSInode* GetTMPFSChild(const char* name) const;
        Inode* GetChild(uint64_t index) const override;
        Inode* GetChild(const char* name) const override;
        uint64_t GetChildCount() const override;
        bool RemoveChild(TempFSInode* child);

        bool SetParent(TempFSInode* parent);
        TempFSInode* GetParent() const override;

        void ResetID(uint32_t seed = 0) override;

        FilePrivilegeLevel GetPrivilegeLevel() const override;
        void SetPrivilegeLevel(FilePrivilegeLevel privilege) override;

        size_t GetSize() const;

        void SetCurrentHead(Head head);
        Head GetCurrentHead() const;

    protected:

        void SetLastError(InodeError error) const override;

        TempFSInode* GetTarget(); // resolve the actual target of an operation. for files and folders, it just returns `this`, but for symbolic links, it returns the sub inode.
        const TempFSInode* GetTarget() const;

    private:
        TempFSInode* m_parent;
        TempFileSystem* m_fileSystem;
        FilePrivilegeLevel m_privilegeLevel;

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