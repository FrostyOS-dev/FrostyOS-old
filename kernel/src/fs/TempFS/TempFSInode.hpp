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

#include <spinlock.h>
#include <util.h>

#include <Data-structures/LinkedList.hpp>


namespace TempFS {
    class TempFileSystem;

    class TempFSInode : public Inode {
    public:
        struct Head {
            int64_t CurrentOffset;
            bool isOpen;
            
            void* currentBlock;
            int64_t currentBlockIndex;
            size_t currentBlockOffset;
        };

        

        TempFSInode();
        ~TempFSInode() override;

        int Create(const char* name, TempFSInode* parent, InodeType type, TempFileSystem* fileSystem, FilePrivilegeLevel privilege, size_t blockSize = PAGE_SIZE, void* extra = nullptr, uint32_t seed = 0);
        int Delete(bool delete_target = false, bool delete_name = false); // normally just deletes this and not the target

        int Open() override;
        int Close() override;
        int64_t ReadStream(FilePrivilegeLevel privilege, uint8_t* bytes, int64_t count = 1, int* status = nullptr) override; // status will be set if not nullptr, and if the return value is >= 0
        int64_t WriteStream(FilePrivilegeLevel privilege, const uint8_t* bytes, int64_t count = 1, int* status = nullptr) override; // status will be set if not nullptr, and if the return value is >= 0
        int Seek(int64_t offset) override;
        int Rewind() override;

        int64_t Read(FilePrivilegeLevel privilege, int64_t offset, uint8_t* bytes, int64_t count = 1, int* status = nullptr) override; // status will be set if not nullptr, and if the return value is >= 0
        int64_t Write(FilePrivilegeLevel privilege, int64_t offset, const uint8_t* bytes, int64_t count = 1, int* status = nullptr) override; // status will be set if not nullptr, and if the return value is >= 0

        int Expand(size_t new_size) override;

        InodeType GetType() const override;
        void SetType(InodeType type) override;

        int AddChild(TempFSInode* child);
        TempFSInode* GetTMPFSChild(uint64_t ID, int* status = nullptr) const; // status will be set if not nullptr
        TempFSInode* GetTMPFSChild(const char* name, int* status = nullptr) const; // status will be set if not nullptr
        Inode* GetChild(uint64_t index, int* status = nullptr) const override; // status will be set if not nullptr
        Inode* GetChild(const char* name, int* status = nullptr) const override; // status will be set if not nullptr
        uint64_t GetChildCount() const override;
        int RemoveChild(TempFSInode* child);

        int SetParent(TempFSInode* parent);
        TempFSInode* GetParent(int* status = nullptr) const override; // status will be set if not nullptr

        void ResetID(uint32_t seed = 0) override;

        FilePrivilegeLevel GetPrivilegeLevel() const override;
        void SetPrivilegeLevel(FilePrivilegeLevel privilege) override;

        size_t GetSize(int* status = nullptr) const; // status will be set if not nullptr

        void SetCurrentHead(Head head);
        Head GetCurrentHead() const;

        void Lock() const override;
        void Unlock() const override;

    protected:

        TempFSInode* GetTarget(int* status = nullptr); // resolve the actual target of an operation. for files and folders, it just returns `this`, but for symbolic links, it returns the sub inode. status will be set if not nullptr
        const TempFSInode* GetTarget(int* status = nullptr) const; // status will be set if not nullptr

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

        mutable spinlock_t m_lock;
    };
}

#endif /* _TEMP_FS_INODE_HPP */