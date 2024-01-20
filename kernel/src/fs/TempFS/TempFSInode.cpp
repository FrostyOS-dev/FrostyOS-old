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

#include <Memory/PageManager.hpp>

namespace TempFS {
    TempFSInode::TempFSInode() {

    }
    
    TempFSInode::~TempFSInode() {

    }

    bool TempFSInode::Create(const char* name, TempFSInode* parent, InodeType type, TempFileSystem* fileSystem, FilePrivilegeLevel privilege, size_t blockSize, void* extra, uint32_t seed) {
        if (name == nullptr || fileSystem == nullptr || blockSize == 0 || type == InodeType::Unkown || (type == InodeType::SymLink && extra == nullptr)) {
            SetLastError(InodeError::INVALID_ARGUMENTS);
            return false;
        }
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
            if (!m_parent->AddChild(this))
                return false;
        }
        else
            m_fileSystem->CreateNewRootInode(this);
        if (p_type == InodeType::SymLink)
            m_children.insert((TempFSInode*)extra);
        SetLastError(InodeError::SUCCESS);
        return true;
    }

    bool TempFSInode::Delete(bool delete_target, bool delete_name) {
        TempFSInode* target = GetTarget();
        if (target != this && delete_target) {
            if (target == nullptr)
                return false;
            bool rc = target->Delete();
            if (!rc)
                return false;
        }
        for (uint64_t i = GetChildCount(); i > 0; i--) {
            TempFSInode* child = m_children.get(i - 1);
            if (child == nullptr) {
                SetLastError(InodeError::INTERNAL_ERROR);
                return false;
            }
            child->Delete(delete_name);
            delete child;
        }
        if (m_parent != nullptr) {
            if (!m_parent->RemoveChild(this)) {
                if (GetLastError() == InodeError::INVALID_ARGUMENTS)
                    SetLastError(InodeError::INTERNAL_ERROR);
                return false;
            }
        }
        else
            m_fileSystem->DeleteRootInode(this);
        p_isOpen = false; // Inlined from Close()
        if (p_type == InodeType::File) {
            for (uint64_t i = 0; i < m_data.getCount(); i++) {
                MemBlock* block = m_data.get(i);
                if (block == nullptr) {
                    SetLastError(InodeError::INTERNAL_ERROR);
                    return false;
                }
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
        SetLastError(InodeError::SUCCESS);
        return true;
    }
    
    bool TempFSInode::Open() {
        TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr)
                return false;
            return target->Open();
        }
        if (p_isOpen) {
            SetLastError(InodeError::SUCCESS);
            return true; // already open
        }
        if (p_type != InodeType::File) {
            SetLastError(InodeError::INVALID_TYPE);
            return false;
        }
        p_isOpen = true;
        if (!Rewind())
            return false;
        SetLastError(InodeError::SUCCESS);
        return true;
    }
    
    bool TempFSInode::Close() {
        TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr)
                return false;
            return target->Close();
        }
        if (!p_isOpen) {
            SetLastError(InodeError::SUCCESS);
            return true; // already closed
        }
        if (p_type != InodeType::File) {
            SetLastError(InodeError::INVALID_TYPE);
            return false;
        }
        p_isOpen = false;
        SetLastError(InodeError::SUCCESS);
        return true;
    }
    
    uint64_t TempFSInode::ReadStream(FilePrivilegeLevel privilege, uint8_t* bytes, uint64_t count) {
        TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr) // error is already set by GetTarget()
                return 0;
            return target->ReadStream(privilege, bytes, count);
        }
        if (!p_isOpen) {
            SetLastError(InodeError::STREAM_CLOSED);
            return 0;
        }
        if (p_type != InodeType::File) {
            SetLastError(InodeError::INVALID_TYPE);
            return 0;
        }
        if (bytes == nullptr || count == 0) {
            SetLastError(InodeError::INVALID_ARGUMENTS);
            return 0;
        }
        if (privilege.UID == m_privilegeLevel.UID || privilege.UID == 0) {
            if (!((m_privilegeLevel.ACL & ACL_USER_READ) > 0)) {
                SetLastError(InodeError::NO_PERMISSION);
                return 0;
            }
        }
        else if (privilege.GID == m_privilegeLevel.GID || privilege.GID == 0) {
            if (!((m_privilegeLevel.ACL & ACL_GROUP_READ) > 0)) {
                SetLastError(InodeError::NO_PERMISSION);
                return 0;
            }
        }
        else {
            if (!((m_privilegeLevel.ACL & ACL_OTHER_READ) > 0)) {
                SetLastError(InodeError::NO_PERMISSION);
                return 0;
            }
        }
        uint16_t bytes_read = 0;
        for (uint64_t currentCount = 0; currentCount < count; m_currentBlockIndex++) {
            if ((p_CurrentOffset + bytes_read) >= m_size) {
                SetLastError(InodeError::INVALID_ARGUMENTS);
                return bytes_read;
            }
            m_currentBlock = m_data.get(m_currentBlockIndex);
            if (m_currentBlock == nullptr) {
                p_CurrentOffset += currentCount;
                SetLastError(InodeError::INTERNAL_ERROR);
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
        SetLastError(InodeError::SUCCESS);
        return bytes_read;
    }
    
    uint64_t TempFSInode::WriteStream(FilePrivilegeLevel privilege, const uint8_t* bytes, uint64_t count) {
        TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr) // error is already set by GetTarget()
                return 0;
            return target->WriteStream(privilege, bytes, count);
        }
        if (!p_isOpen) {
            SetLastError(InodeError::STREAM_CLOSED);
            return 0;
        }
        if (p_type != InodeType::File) {
            SetLastError(InodeError::INVALID_TYPE);
            return 0;
        }
        if (bytes == nullptr || count == 0) {
            SetLastError(InodeError::INVALID_ARGUMENTS);
            return 0;
        }
        if (privilege.UID == m_privilegeLevel.UID || privilege.UID == 0) {
            if (!((m_privilegeLevel.ACL & ACL_USER_WRITE) > 0)) {
                SetLastError(InodeError::NO_PERMISSION);
                return 0;
            }
        }
        else if (privilege.GID == m_privilegeLevel.GID || privilege.GID == 0) {
            if (!((m_privilegeLevel.ACL & ACL_GROUP_WRITE) > 0)) {
                SetLastError(InodeError::NO_PERMISSION);
                return 0;
            }
        }
        else {
            if (!((m_privilegeLevel.ACL & ACL_OTHER_WRITE) > 0)) {
                SetLastError(InodeError::NO_PERMISSION);
                return 0;
            }
        }
        uint64_t bytes_written = 0;
        for (uint64_t currentCount = 0; currentCount < count; m_currentBlockIndex++) {
            m_currentBlock = m_data.get(m_currentBlockIndex);
            if (m_currentBlock == nullptr) {
                if (!Expand(count)) { // error is already set by Expand()
                    p_CurrentOffset += currentCount;
                    return bytes_written;
                }
                m_currentBlock = m_data.get(m_currentBlockIndex);
                if (m_currentBlock == nullptr) {
                    SetLastError(InodeError::INTERNAL_ERROR);
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
        SetLastError(InodeError::SUCCESS);
        return bytes_written;
    }
    
    bool TempFSInode::Seek(uint64_t offset) {
        TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr)
                return false;
            return target->Seek(offset);
        }
        if (!p_isOpen) {
            SetLastError(InodeError::STREAM_CLOSED);
            return false;
        }
        if (p_type != InodeType::File) {
            SetLastError(InodeError::INVALID_TYPE);
            return false;
        }
        if (offset >= m_size) {
            SetLastError(InodeError::INVALID_ARGUMENTS);
            return false;
        }
        offset = ALIGN_UP(offset, 8);
        p_CurrentOffset = 0;
        for (; m_currentBlockIndex < m_data.getCount(); m_currentBlockIndex++) {
            m_currentBlock = m_data.get(m_currentBlockIndex);
            if (m_currentBlock == nullptr) {
                SetLastError(InodeError::INTERNAL_ERROR);
                return false;
            }
            if ((m_currentBlock->size + p_CurrentOffset) > offset) {
                m_currentBlockOffset = offset - p_CurrentOffset;
                p_CurrentOffset += m_currentBlockOffset;
                break;
            }
            p_CurrentOffset += m_currentBlock->size;
        }
        SetLastError(InodeError::SUCCESS);
        return true;
    }
    
    bool TempFSInode::Rewind() {
        TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr)
                return false;
            return target->Rewind();
        }
        if (!p_isOpen) {
            SetLastError(InodeError::STREAM_CLOSED);
            return false;
        }
        if (p_type != InodeType::File) {
            SetLastError(InodeError::INVALID_TYPE);
            return false;
        }
        m_currentBlockIndex = 0;
        m_currentBlockOffset = 0;
        p_CurrentOffset = 0;
        m_currentBlock = m_data.get(0);
        SetLastError(InodeError::SUCCESS);
        return true;
    }
    

    uint64_t TempFSInode::Read(FilePrivilegeLevel privilege, uint64_t offset, uint8_t* bytes, uint64_t count) {
        TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr) // error is already set by GetTarget()
                return 0;
            return target->Read(privilege, offset, bytes, count);
        }
        bool isOpen = p_isOpen;
        if (!p_isOpen) {
            if (!Open())
                return 0;
        }
        if (!Seek(offset))
            return 0;
        uint64_t status = ReadStream(privilege, bytes, count);
        if (!isOpen)
            Close(); // no need to check return value. error is set and the caller is responsible for checking it.
        return status;
    }
    
    uint64_t TempFSInode::Write(FilePrivilegeLevel privilege, uint64_t offset, const uint8_t* bytes, uint64_t count) {
        TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr) // error is already set by GetTarget()
                return 0;
            return target->Write(privilege, offset, bytes, count);
        }
        bool isOpen = p_isOpen;
        if (!p_isOpen) {
            if (!Open())
                return 0;
        }
        if (!Seek(offset))
            return 0;
        uint64_t status = WriteStream(privilege, bytes, count);
        if (!isOpen)
            Close();
        return status;
    }

    bool TempFSInode::Expand(size_t new_size) {
        TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr)
                return false;
            return target->Expand(new_size);
        }
        MemBlock* mem_block = new MemBlock;
        if (mem_block == nullptr) {
            SetLastError(InodeError::ALLOCATION_FAILED);
            return false;
        }
        mem_block->size = ALIGN_UP((new_size - m_size), p_blockSize);
        mem_block->address = g_KPM->AllocatePages(mem_block->size / PAGE_SIZE);
        if ((mem_block->address) == nullptr) {
            SetLastError(InodeError::ALLOCATION_FAILED);
            return false;
        }
        m_size = new_size;
        m_data.insert(mem_block);
        SetLastError(InodeError::SUCCESS);
        return true;
    }

    InodeType TempFSInode::GetType() const {
        switch (p_type) {
        case InodeType::File:
        case InodeType::Folder:
            SetLastError(InodeError::SUCCESS);
            return p_type;
        case InodeType::SymLink:
        {
            TempFSInode* sub = m_children.get(0);
            if (sub == nullptr) {
                SetLastError(InodeError::INTERNAL_ERROR);
                return InodeType::Unkown;
            }
            SetLastError(InodeError::SUCCESS);
            return sub->GetType();
        }
        default:
            SetLastError(InodeError::INVALID_TYPE);
            return InodeType::Unkown;
        }
        SetLastError(InodeError::INTERNAL_ERROR); // should be unreachable
        return InodeType::Unkown;
    }

    void TempFSInode::SetType(InodeType type) {
        switch (p_type) {
        case InodeType::File:
        case InodeType::Folder:
            SetLastError(InodeError::SUCCESS);
            p_type = type;
            return;
        case InodeType::SymLink:
        {
            TempFSInode* sub = m_children.get(0);
            if (sub == nullptr)
                SetLastError(InodeError::INTERNAL_ERROR);
            SetLastError(InodeError::SUCCESS);
            return sub->SetType(type);
        }
        default:
            SetLastError(InodeError::INVALID_TYPE);
            return;
        }
        SetLastError(InodeError::INTERNAL_ERROR); // should be unreachable
    }

    bool TempFSInode::AddChild(TempFSInode* child) {
        TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr)
                return false;
            return target->AddChild(child);
        }
        if (child == nullptr) {
            SetLastError(InodeError::INVALID_ARGUMENTS);
            return false;
        }
        if (p_type != InodeType::Folder) {
            SetLastError(InodeError::INVALID_TYPE);
            return false;
        }
        m_children.insert(child);
        SetLastError(InodeError::SUCCESS);
        return true;
    }

    TempFSInode* TempFSInode::GetTMPFSChild(uint64_t ID) const {
        const TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr)
                return nullptr;
            return target->GetTMPFSChild(ID);
        }
        if (p_type != InodeType::Folder) {
            SetLastError(InodeError::INVALID_TYPE);
            return nullptr;
        }
        for (uint64_t i = 0; i < m_children.getCount(); i++) {
            TempFSInode* inode = m_children.get(i);
            if (inode == nullptr) {
                SetLastError(InodeError::INTERNAL_ERROR);
                return nullptr;
            }
            if (inode->p_ID == ID) {
                SetLastError(InodeError::SUCCESS);
                return inode;
            }
        }
        SetLastError(InodeError::INVALID_ARGUMENTS);
        return nullptr;
    }

    TempFSInode* TempFSInode::GetTMPFSChild(const char* name) const {
        const TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr)
                return nullptr;
            return target->GetTMPFSChild(name);
        }
        if (p_type != InodeType::Folder) {
            SetLastError(InodeError::INVALID_TYPE);
            return nullptr;
        }
        if (name == nullptr) {
            SetLastError(InodeError::INVALID_ARGUMENTS);
            return nullptr;
        }
        for (uint64_t i = 0; i < m_children.getCount(); i++) {
            TempFSInode* inode = m_children.get(i);
            if (inode == nullptr) {
                SetLastError(InodeError::INTERNAL_ERROR);
                return nullptr;
            }
            if (strcmp(name, inode->p_name) == 0) {
                SetLastError(InodeError::SUCCESS);
                return inode;
            }
        }
        SetLastError(InodeError::INVALID_ARGUMENTS);
        return nullptr;
    }

    Inode* TempFSInode::GetChild(const char* name) const {
        const TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr)
                return nullptr;
            return target->GetChild(name);
        }
        if (p_type != InodeType::Folder) {
            SetLastError(InodeError::INVALID_TYPE);
            return nullptr;
        }
        if (name == nullptr) {
            SetLastError(InodeError::INVALID_ARGUMENTS);
            return nullptr;
        }
        for (uint64_t i = 0; i < m_children.getCount(); i++) {
            TempFSInode* inode = m_children.get(i);
            if (inode == nullptr) {
                SetLastError(InodeError::INTERNAL_ERROR);
                return nullptr;
            }
            if (strcmp(name, inode->p_name) == 0) {
                SetLastError(InodeError::SUCCESS);
                return (Inode*)inode;
            }
        }
        SetLastError(InodeError::INVALID_ARGUMENTS);
        return nullptr;
    }

    Inode* TempFSInode::GetChild(uint64_t index) const {
        const TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr)
                return nullptr;
            return target->GetChild(index);
        }
        if (p_type != InodeType::Folder) {
            SetLastError(InodeError::INVALID_TYPE);
            return nullptr;
        }
        if (index >= m_children.getCount()) {
            SetLastError(InodeError::INVALID_ARGUMENTS);
            return nullptr;
        }
        TempFSInode* inode = m_children.get(index);
        if (inode == nullptr) {
            SetLastError(InodeError::INTERNAL_ERROR);
            return nullptr;
        }
        SetLastError(InodeError::SUCCESS);
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

    bool TempFSInode::RemoveChild(TempFSInode* child) {
        TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr)
                return false;
            return target->RemoveChild(child);
        }
        if (p_type != InodeType::Folder) {
            SetLastError(InodeError::INVALID_TYPE);
            return false;
        }
        if (child == nullptr) {
            SetLastError(InodeError::INVALID_ARGUMENTS);
            return false;
        }
        uint64_t index = m_children.getIndex(child);
        if (index == UINT64_MAX) {
            SetLastError(InodeError::INVALID_ARGUMENTS);
            return false;
        }
        m_children.remove(index); // Index-based removal is used to ensure that removal is successful, and correct error reporting occurs
        SetLastError(InodeError::SUCCESS);
        return true;
    }

    bool TempFSInode::SetParent(TempFSInode* parent) {
        TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr)
                return false;
            return target->SetParent(parent);
        }
        if (m_parent != nullptr) {
            if (!m_parent->RemoveChild(this)) {
                if (GetLastError() == InodeError::INVALID_ARGUMENTS)
                    SetLastError(InodeError::INTERNAL_ERROR);
                return false;
            }
        }
        else
            m_fileSystem->DeleteRootInode(this);
        m_parent = parent;
        if (m_parent != nullptr)
            m_parent->AddChild(this);
        else
            m_fileSystem->CreateNewRootInode(this);
        SetLastError(InodeError::SUCCESS);
        return true;
    }

    TempFSInode* TempFSInode::GetParent() const {
        const TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr)
                return nullptr;
            return target->GetParent();
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


    void TempFSInode::SetLastError(InodeError error) const {
        p_lastError = error;
    }

    TempFSInode* TempFSInode::GetTarget() {
        if (p_type == InodeType::SymLink) {
            TempFSInode* sub = m_children.get(0);
            if (sub == nullptr)
                SetLastError(InodeError::INTERNAL_ERROR);
            else
                SetLastError(InodeError::SUCCESS);
            return sub;
        }
        else {
            SetLastError(InodeError::SUCCESS);
            return this;
        }
    }

    const TempFSInode* TempFSInode::GetTarget() const {
        if (p_type == InodeType::SymLink) {
            TempFSInode* sub = m_children.get(0);
            if (sub == nullptr || sub->GetID() == 0)
                SetLastError(InodeError::INTERNAL_ERROR);
            else
                SetLastError(InodeError::SUCCESS);
            return sub;
        }
        else {
            SetLastError(InodeError::SUCCESS);
            return this;
        }
    }

    FilePrivilegeLevel TempFSInode::GetPrivilegeLevel() const {
        return m_privilegeLevel;
    }

    void TempFSInode::SetPrivilegeLevel(FilePrivilegeLevel privilege) {
        m_privilegeLevel = privilege;
    }

    size_t TempFSInode::GetSize() const {
        const TempFSInode* target = GetTarget();
        if (target != this) {
            if (target == nullptr) {
                SetLastError(InodeError::INTERNAL_ERROR);
                return 0;
            }
            return target->GetSize();
        }
        if (GetType() != InodeType::File) {
            SetLastError(InodeError::INVALID_TYPE);
            return 0;
        }
        return m_size;
    }
}
