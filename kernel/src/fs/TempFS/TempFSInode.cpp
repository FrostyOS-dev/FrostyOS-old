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

#include "TempFSInode.hpp"
#include "TempFileSystem.hpp"

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <Memory/PageManager.hpp>

namespace TempFS {
    TempFSInode::TempFSInode() {

    }
    
    TempFSInode::~TempFSInode() {

    }

    int TempFSInode::Create(const char* name, TempFSInode* parent, InodeType type, TempFileSystem* fileSystem, FilePrivilegeLevel privilege, size_t blockSize, void* extra, uint32_t seed) {
        if (name == nullptr || fileSystem == nullptr || blockSize == 0 || type == InodeType::Unkown || (type == InodeType::SymLink && extra == nullptr))
            return -EINVAL;
        m_parent = parent;
        m_fileSystem = fileSystem;
        m_privilegeLevel = privilege;
        m_size = 0;
        p_name = name;
        p_type = type;
        p_blockSize = blockSize;
        p_CurrentOffset = 0;
        ResetID(seed);
        if (m_parent != nullptr) {
            int rc = m_parent->AddChild(this);
            if (rc != ESUCCESS)
                return rc;
        }
        else
            m_fileSystem->CreateNewRootInode(this);
        if (p_type == InodeType::SymLink)
            m_children.insert((TempFSInode*)extra);
        return ESUCCESS;
    }

    int TempFSInode::Delete(bool delete_target, bool delete_name) {
        TempFSInode* target = GetTarget();
        if (target != this && delete_target) {
            if (target == nullptr)
                return -ENOLINK;
            int rc = target->Delete();
            if (rc < 0)
                return rc;
        }
        for (uint64_t i = GetChildCount(); i > 0; i--) {
            TempFSInode* child = m_children.get(i - 1);
            if (child == nullptr)
                return -ENOSYS;
            child->Delete(delete_name);
            delete child;
        }
        if (m_parent != nullptr) {
            int rc = m_parent->RemoveChild(this);
            if (rc == -EINVAL)
                return -ENOSYS;
        }
        else
            m_fileSystem->DeleteRootInode(this);
        p_isOpen = false; // Inlined from Close()
        if (p_type == InodeType::File) {
            for (uint64_t i = 0; i < m_data.getCount(); i++) {
                MemBlock* block = m_data.get(i);
                if (block == nullptr)
                    return -ENOSYS;
                uint64_t pages = block->size / PAGE_SIZE;
                if (pages > 1)
                    g_KPM->FreePages(block->address);
                else
                    g_KPM->FreePage(block->address);
                delete block; 
            }
        }
        if (delete_name)
            delete[] p_name;
        p_ID = 0; // invalidate this inode
        return ESUCCESS;
    }
    
    int TempFSInode::Open() {
        TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr)
                return -ENOLINK;
            return target->Open();
        }
        if (p_isOpen)
            return ESUCCESS; // already open
        if (p_type != InodeType::File)
            return -EISDIR;
        p_isOpen = true;
        int rc = Rewind();
        if (rc != ESUCCESS)
            return rc;
        return ESUCCESS;
    }
    
    int TempFSInode::Close() {
        TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr)
                return -ENOLINK;
            return target->Close();
        }
        if (!p_isOpen)
            return ESUCCESS; // already closed
        if (p_type != InodeType::File)
            return -EISDIR;
        p_isOpen = false;
        return ESUCCESS;
    }
    
    int64_t TempFSInode::ReadStream(FilePrivilegeLevel privilege, uint8_t* bytes, int64_t count, int* status) {
        TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr) // error is already set by GetTarget()
                return -ENOLINK;
            return target->ReadStream(privilege, bytes, count, status);
        }
        if (!p_isOpen)
            return -EBADF;
        if (p_type != InodeType::File)
            return -EISDIR;
        if (bytes == nullptr || count == 0)
            return -EINVAL;
        if (privilege.UID == m_privilegeLevel.UID || privilege.UID == 0) {
            if (!((m_privilegeLevel.ACL & ACL_USER_READ) > 0))
                return -EACCES;
        }
        else if (privilege.GID == m_privilegeLevel.GID || privilege.GID == 0) {
            if (!((m_privilegeLevel.ACL & ACL_GROUP_READ) > 0))
                return -EACCES;
        }
        else {
            if (!((m_privilegeLevel.ACL & ACL_OTHER_READ) > 0))
                return -EACCES;
        }
        uint16_t bytes_read = 0;
        for (uint64_t currentCount = 0; currentCount < count; m_currentBlockIndex++) {
            if ((p_CurrentOffset + bytes_read) >= m_size) {
                if (status != nullptr)
                    *status = -EINVAL;
                return bytes_read;
            }
            m_currentBlock = m_data.get(m_currentBlockIndex);
            if (m_currentBlock == nullptr) {
                p_CurrentOffset += currentCount;
                if (status != nullptr)
                    *status = -ENOSYS;
                return bytes_read;
            }
            if ((m_currentBlock->size - m_currentBlockOffset + currentCount) > count) {
                memcpy((void*)((uint64_t)bytes + currentCount), (void*)((uint64_t)(m_currentBlock->address) + m_currentBlockOffset), count - currentCount);
                m_currentBlockOffset += count - currentCount;
                bytes_read += count - currentCount;
                break;
            }
            memcpy((void*)((uint64_t)bytes + currentCount), (void*)((uint64_t)m_currentBlock->address + m_currentBlockOffset), m_currentBlock->size - (m_currentBlockOffset + 1));
            currentCount += m_currentBlock->size - m_currentBlockOffset;
            bytes_read += m_currentBlock->size - (m_currentBlockOffset + 1);
            m_currentBlockOffset = 0;
        }
        p_CurrentOffset += count;
        if (status != nullptr)
            *status = ESUCCESS;
        return bytes_read;
    }
    
    int64_t TempFSInode::WriteStream(FilePrivilegeLevel privilege, const uint8_t* bytes, int64_t count, int* status) {
        TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr) // error is already set by GetTarget()
                return -ENOLINK;
            return target->WriteStream(privilege, bytes, count, status);
        }
        if (!p_isOpen)
            return -EBADF;
        if (p_type != InodeType::File)
            return -EISDIR;
        if (bytes == nullptr || count == 0)
            return -EINVAL;
        if (privilege.UID == m_privilegeLevel.UID || privilege.UID == 0) {
            if (!((m_privilegeLevel.ACL & ACL_USER_WRITE) > 0))
                return -EACCES;
        }
        else if (privilege.GID == m_privilegeLevel.GID || privilege.GID == 0) {
            if (!((m_privilegeLevel.ACL & ACL_GROUP_WRITE) > 0))
                return -EACCES;
        }
        else {
            if (!((m_privilegeLevel.ACL & ACL_OTHER_WRITE) > 0))
                return -EACCES;
        }
        uint64_t bytes_written = 0;
        for (uint64_t currentCount = 0; currentCount < count; m_currentBlockIndex++) {
            m_currentBlock = m_data.get(m_currentBlockIndex);
            if (m_currentBlock == nullptr) {
                int rc = Expand(count);
                if (rc < 0) {
                    p_CurrentOffset += currentCount;
                    if (status != nullptr)
                        *status = rc;
                    return bytes_written;
                }
                m_currentBlock = m_data.get(m_currentBlockIndex);
                if (m_currentBlock == nullptr) {
                    if (status != nullptr)
                        *status = -ENOSYS;
                    return bytes_written;
                }
            }
            if ((m_currentBlock->size - m_currentBlockOffset + currentCount) > count) {
                memcpy((void*)((uint64_t)(m_currentBlock->address) + m_currentBlockOffset), (void*)((uint64_t)bytes + currentCount), count - currentCount);
                m_currentBlockOffset += count - currentCount;
                bytes_written += count - currentCount;
                break;
            }
            memcpy((void*)((uint64_t)m_currentBlock->address + m_currentBlockOffset), (void*)((uint64_t)bytes + currentCount), m_currentBlock->size - (m_currentBlockOffset + 1));
            currentCount += m_currentBlock->size - m_currentBlockOffset;
            bytes_written += m_currentBlock->size - (m_currentBlockOffset + 1);
            m_currentBlockOffset = 0;
        }
        p_CurrentOffset += count;
        if (status != nullptr)
            *status = ESUCCESS;
        return bytes_written;
    }
    
    int TempFSInode::Seek(int64_t offset) {
        TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr)
                return -ENOLINK;
            return target->Seek(offset);
        }
        if (!p_isOpen)
            return -EBADF;
        if (p_type != InodeType::File)
            return -EISDIR;
        if (offset >= m_size)
            return -EINVAL;
        offset = ALIGN_UP(offset, 8);
        p_CurrentOffset = 0;
        for (; m_currentBlockIndex < m_data.getCount(); m_currentBlockIndex++) {
            m_currentBlock = m_data.get(m_currentBlockIndex);
            if (m_currentBlock == nullptr)
                return -ENOSYS;
            if ((m_currentBlock->size + p_CurrentOffset) > offset) {
                m_currentBlockOffset = offset - p_CurrentOffset;
                p_CurrentOffset += m_currentBlockOffset;
                break;
            }
            p_CurrentOffset += m_currentBlock->size;
        }
        return ESUCCESS;
    }
    
    int TempFSInode::Rewind() {
        TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr)
                return -ENOLINK;
            return target->Rewind();
        }
        if (!p_isOpen)
            return -EBADF;
        if (p_type != InodeType::File)
            return -EISDIR;
        m_currentBlockIndex = 0;
        m_currentBlockOffset = 0;
        p_CurrentOffset = 0;
        m_currentBlock = m_data.get(0);
        return ESUCCESS;
    }
    

    int64_t TempFSInode::Read(FilePrivilegeLevel privilege, int64_t offset, uint8_t* bytes, int64_t count, int* status) {
        TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr) // error is already set by GetTarget()
                return -ENOLINK;
            return target->Read(privilege, offset, bytes, count, status);
        }
        bool isOpen = p_isOpen;
        if (!p_isOpen) {
            int rc = Open();
            if (rc < 0)
                return rc;
        }
        int rc = Seek(offset);
        if (rc < 0) {
            if (!isOpen)
                Close(); // no need to check return value. error already occurred, this is just cleanup
            return rc;
        }
        int rs_status;
        int64_t i_status = ReadStream(privilege, bytes, count, &rs_status);
        if (!isOpen)
            Close(); // no need to check return value. error is set and the caller is responsible for checking it.
        if (status != nullptr)
            *status = rs_status;
        return i_status;
    }
    
    int64_t TempFSInode::Write(FilePrivilegeLevel privilege, int64_t offset, const uint8_t* bytes, int64_t count, int* status) {
        TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr) // error is already set by GetTarget()
                return -ENOLINK;
            return target->Write(privilege, offset, bytes, count, status);
        }
        bool isOpen = p_isOpen;
        if (!p_isOpen) {
            int rc = Open();
            if (rc < 0)
                return rc;
        }
        int rc = Seek(offset);
        if (rc < 0) {
            if (!isOpen)
                Close(); // no need to check return value. error already occurred, this is just cleanup
            return rc;
        }
        int rs_status;
        int64_t i_status = WriteStream(privilege, bytes, count, &rs_status);
        if (!isOpen)
            Close();
        if (status != nullptr)
            *status = rs_status;
        return i_status;
    }

    int TempFSInode::Expand(size_t new_size) {
        TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr)
                return -ENOLINK;
            return target->Expand(new_size);
        }
        MemBlock* mem_block = new MemBlock;
        if (mem_block == nullptr)
            return -ENOMEM;
        mem_block->size = ALIGN_UP((new_size - m_size), p_blockSize);
        mem_block->address = g_KPM->AllocatePages(mem_block->size / PAGE_SIZE);
        if ((mem_block->address) == nullptr)
            return -ENOMEM;
        m_size = new_size;
        m_data.insert(mem_block);
        return ESUCCESS;
    }

    InodeType TempFSInode::GetType() const {
        switch (p_type) {
        case InodeType::File:
        case InodeType::Folder:
            return p_type;
        case InodeType::SymLink:
        {
            TempFSInode* sub = m_children.get(0);
            if (sub == nullptr)
                return InodeType::Unkown;
            return sub->GetType();
        }
        default:
            return InodeType::Unkown;
        }
        return InodeType::Unkown;
    }

    void TempFSInode::SetType(InodeType type) {
        switch (p_type) {
        case InodeType::File:
        case InodeType::Folder:
            p_type = type;
            return;
        case InodeType::SymLink:
        {
            TempFSInode* sub = m_children.get(0);
            return sub->SetType(type);
        }
        default:
            return;
        }
    }

    int TempFSInode::AddChild(TempFSInode* child) {
        TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr)
                return -ENOLINK;
            return target->AddChild(child);
        }
        if (child == nullptr)
            return -EFAULT;
        if (p_type != InodeType::Folder)
            return -ENOTDIR;
        m_children.insert(child);
        return ESUCCESS;
    }

    TempFSInode* TempFSInode::GetTMPFSChild(uint64_t ID, int* status) const {
        const TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr) {
                if (status != nullptr)
                    *status = -ENOLINK;
                return nullptr;
            }
            return target->GetTMPFSChild(ID, status);
        }
        if (p_type != InodeType::Folder) {
            if (status != nullptr)
                *status = -ENOTDIR;
            return nullptr;
        }
        for (uint64_t i = 0; i < m_children.getCount(); i++) {
            TempFSInode* inode = m_children.get(i);
            if (inode == nullptr) {
                if (status != nullptr)
                    *status = -ENOSYS;
                return nullptr;
            }
            if (inode->p_ID == ID)
                return inode;
        }
        if (status != nullptr)
            *status = -ENOENT;
        return nullptr;
    }

    TempFSInode* TempFSInode::GetTMPFSChild(const char* name, int* status) const {
        const TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr) {
                if (status != nullptr)
                    *status = -ENOLINK;
                return nullptr;
            }
            return target->GetTMPFSChild(name, status);
        }
        if (p_type != InodeType::Folder) {
            if (status != nullptr)
                *status = -ENOTDIR;
            return nullptr;
        }
        if (name == nullptr) {
            if (status != nullptr)
                *status = -EFAULT;
            return nullptr;
        }
        for (uint64_t i = 0; i < m_children.getCount(); i++) {
            TempFSInode* inode = m_children.get(i);
            if (inode == nullptr) {
                if (status != nullptr)
                    *status = -ENOSYS;
                return nullptr;
            }
            if (strcmp(name, inode->p_name) == 0)
                return inode;
        }
        if (status != nullptr)
            *status = -ENOENT;
        return nullptr;
    }

    Inode* TempFSInode::GetChild(const char* name, int* status) const {
        const TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr) {
                if (status != nullptr)
                    *status = -ENOLINK;
                return nullptr;
            }
            return target->GetChild(name, status);
        }
        if (p_type != InodeType::Folder) {
            if (status != nullptr)
                *status = -ENOTDIR;
            return nullptr;
        }
        if (name == nullptr) {
            if (status != nullptr)
                *status = -EFAULT;
            return nullptr;
        }
        for (uint64_t i = 0; i < m_children.getCount(); i++) {
            TempFSInode* inode = m_children.get(i);
            if (inode == nullptr) {
                if (status != nullptr)
                    *status = -ENOSYS;
                return nullptr;
            }
            if (strcmp(name, inode->p_name) == 0)
                return (Inode*)inode;
        }
        if (status != nullptr)
            *status = -ENOENT;
        return nullptr;
    }

    Inode* TempFSInode::GetChild(uint64_t index, int* status) const {
        const TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr) {
                if (status != nullptr)
                    *status = -ENOLINK;
                return nullptr;
            }
            return target->GetChild(index, status);
        }
        if (p_type != InodeType::Folder) {
            if (status != nullptr)
                *status = -ENOTDIR;
            return nullptr;
        }
        if (index >= m_children.getCount()) {
            if (status != nullptr)
                *status = -EINVAL;
            return nullptr;
        }
        TempFSInode* inode = m_children.get(index);
        if (inode == nullptr) {
            if (status != nullptr)
                *status = -ENOSYS;
            return nullptr;
        }
        return (Inode*)inode;
    }

    uint64_t TempFSInode::GetChildCount() const {
        const TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr)
                return 0;
            return target->GetChildCount();
        }
        return m_children.getCount();
    }

    int TempFSInode::RemoveChild(TempFSInode* child) {
        TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr)
                return -ENOLINK;
            return target->RemoveChild(child);
        }
        if (p_type != InodeType::Folder)
            return -ENOTDIR;
        if (child == nullptr)
            return -EFAULT;
        uint64_t index = m_children.getIndex(child);
        if (index == UINT64_MAX)
            return -ENOENT;
        m_children.remove(index); // Index-based removal is used to ensure that removal is successful, and correct error reporting occurs
        return ESUCCESS;
    }

    int TempFSInode::SetParent(TempFSInode* parent) {
        TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr)
                return -ENOLINK;
            return target->SetParent(parent);
        }
        if (m_parent != nullptr) {
            int rc = m_parent->RemoveChild(this);
            if (rc < 0)
                return rc;
        }
        else
            m_fileSystem->DeleteRootInode(this);
        m_parent = parent;
        if (m_parent != nullptr)
            m_parent->AddChild(this);
        else
            m_fileSystem->CreateNewRootInode(this);
        return ESUCCESS;
    }

    TempFSInode* TempFSInode::GetParent(int* status) const {
        const TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr) {
                if (status != nullptr)
                    *status = -ENOLINK;
                return nullptr;
            }
            return target->GetParent(status);
        }
        return m_parent;
    }

    void TempFSInode::ResetID(uint32_t seed) {
        if (seed != 0)
            srand(seed);
        p_ID = ((uint64_t)rand() << 32) | rand();
    }

    void TempFSInode::SetCurrentHead(Head head) {
        p_CurrentOffset = head.CurrentOffset;
        p_isOpen = head.isOpen;
        m_currentBlock = (MemBlock*)head.currentBlock;
        m_currentBlockIndex = head.currentBlockIndex;
        m_currentBlockOffset = head.currentBlockOffset;
    }

    TempFSInode::Head TempFSInode::GetCurrentHead() const {
        return {
            .CurrentOffset = p_CurrentOffset,
            .isOpen = p_isOpen,
            .currentBlock = m_currentBlock,
            .currentBlockIndex = m_currentBlockIndex,
            .currentBlockOffset = m_currentBlockOffset
        };
    }

    TempFSInode* TempFSInode::GetTarget(int* status) {
        if (p_type == InodeType::SymLink) {
            TempFSInode* sub = m_children.get(0);
            if (sub == nullptr && status != nullptr)
                *status = -ENOLINK;
            return sub;
        }
        else
            return this;
    }

    const TempFSInode* TempFSInode::GetTarget(int* status) const {
        if (p_type == InodeType::SymLink) {
            TempFSInode* sub = m_children.get(0);
            if ((sub == nullptr || sub->GetID() == 0) && status != nullptr)
                *status = -ENOLINK;
            return sub;
        }
        else
            return this;
    }

    FilePrivilegeLevel TempFSInode::GetPrivilegeLevel() const {
        return m_privilegeLevel;
    }

    void TempFSInode::SetPrivilegeLevel(FilePrivilegeLevel privilege) {
        m_privilegeLevel = privilege;
    }

    size_t TempFSInode::GetSize(int* status) const {
        const TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr) {
                if (status != nullptr)
                    *status = -ENOLINK;
                return 0;
            }
            return target->GetSize();
        }
        if (GetType() != InodeType::File) {
            if (status != nullptr)
                *status = -EISDIR;
            return 0;
        }
        return m_size;
    }

    void TempFSInode::Lock() const {
        spinlock_acquire(&m_lock);
    }

    void TempFSInode::Unlock() const {
        spinlock_release(&m_lock);
    }
}
