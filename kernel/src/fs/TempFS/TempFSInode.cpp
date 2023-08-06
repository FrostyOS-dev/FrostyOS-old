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

#include "TempFSInode.hpp"
#include "TempFileSystem.hpp"

#include <string.h>

// REMOVE THIS INCLUDE. ONLY FOR malloc and rand/srand
#include <stdlib.h>

#include <Memory/PageManager.hpp>

namespace TempFS {
    TempFSInode::TempFSInode() {

    }
    
    TempFSInode::~TempFSInode() {

    }

    bool TempFSInode::Create(const char* name, TempFSInode* parent, InodeType type, TempFileSystem* fileSystem, size_t blockSize, uint32_t seed) {
        if (name == nullptr || fileSystem == nullptr || blockSize == 0) {
            SetLastError(InodeError::INVALID_ARGUMENTS);
            return false;
        }
        m_parent = parent;
        m_fileSystem = fileSystem;
        m_UID = 0;
        m_GID = 0;
        m_ACL = 00644;
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
        SetLastError(InodeError::SUCCESS);
        return true;
    }

    bool TempFSInode::Delete() {
        for (uint64_t i = GetChildCount(); i > 0; i--) {
            TempFSInode* child = m_children.get(i - 1);
            if (child == nullptr) {
                SetLastError(InodeError::INTERNAL_ERROR);
                return false;
            }
            child->Delete();
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
                    WorldOS::g_KPM->FreePages(block->address);
                else
                    WorldOS::g_KPM->FreePage(block->address);
                delete block; 
            }
        }
        SetLastError(InodeError::SUCCESS);
        return true;
    }
    
    bool TempFSInode::Open() {
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
    
    bool TempFSInode::ReadStream(uint8_t* bytes, uint64_t count) {
        if (!p_isOpen) {
            SetLastError(InodeError::STREAM_CLOSED);
            return false;
        }
        if (p_type != InodeType::File) {
            SetLastError(InodeError::INVALID_TYPE);
            return false;
        }
        if (bytes == nullptr || count > m_size) {
            SetLastError(InodeError::INVALID_ARGUMENTS);
            return false;
        }
        count = ALIGN_UP(count, 8);
        for (uint64_t currentCount = 0; currentCount < count; m_currentBlockIndex++) {
            m_currentBlock = m_data.get(m_currentBlockIndex);
            if (m_currentBlock == nullptr) {
                p_CurrentOffset += currentCount;
                SetLastError(InodeError::INTERNAL_ERROR);
                return false;
            }
            if ((m_currentBlock->size - m_currentBlockOffset + currentCount) > count) {
                fast_memcpy((void*)((uint64_t)bytes + currentCount), (void*)((uint64_t)(m_currentBlock->address) + m_currentBlockOffset), count - currentCount);
                m_currentBlockOffset += count - currentCount;
                break;
            }
            fast_memcpy((void*)((uint64_t)bytes + currentCount), (void*)((uint64_t)m_currentBlock->address + m_currentBlockOffset), m_currentBlock->size - (m_currentBlockOffset + 1));
            currentCount += m_currentBlock->size - m_currentBlockOffset;
            m_currentBlockOffset = 0;
        }
        p_CurrentOffset += count;
        SetLastError(InodeError::SUCCESS);
        return true;
    }
    
    bool TempFSInode::WriteStream(const uint8_t* bytes, uint64_t count) {
        if (!p_isOpen) {
            SetLastError(InodeError::STREAM_CLOSED);
            return false;
        }
        if (p_type != InodeType::File) {
            SetLastError(InodeError::INVALID_TYPE);
            return false;
        }
        if (bytes == nullptr) {
            SetLastError(InodeError::INVALID_ARGUMENTS);
            return false;
        }
        count = ALIGN_UP(count, 8);
        for (uint64_t currentCount = 0; currentCount < count; m_currentBlockIndex++) {
            m_currentBlock = m_data.get(m_currentBlockIndex);
            if (m_currentBlock == nullptr) {
                if (!Expand(count)) {
                    p_CurrentOffset += currentCount;
                    return false;
                }
                m_currentBlock = m_data.get(m_currentBlockIndex);
                if (m_currentBlock == nullptr) {
                    SetLastError(InodeError::INTERNAL_ERROR);
                    return false;
                }
            }
            if ((m_currentBlock->size - m_currentBlockOffset + currentCount) > count) {
                fast_memcpy((void*)((uint64_t)(m_currentBlock->address) + m_currentBlockOffset), (void*)((uint64_t)bytes + currentCount), count - currentCount);
                m_currentBlockOffset += count - currentCount;
                break;
            }
            fast_memcpy((void*)((uint64_t)m_currentBlock->address + m_currentBlockOffset), (void*)((uint64_t)bytes + currentCount), m_currentBlock->size - (m_currentBlockOffset + 1));
            currentCount += m_currentBlock->size - m_currentBlockOffset;
            m_currentBlockOffset = 0;
        }
        p_CurrentOffset += count;
        SetLastError(InodeError::SUCCESS);
        return true;
    }
    
    bool TempFSInode::Seek(uint64_t offset) {
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
                m_currentBlockOffset = (m_currentBlock->size + p_CurrentOffset) - offset;
                p_CurrentOffset += m_currentBlockOffset;
                break;
            }
            p_CurrentOffset += m_currentBlock->size;
        }
        SetLastError(InodeError::SUCCESS);
        return true;
    }
    
    bool TempFSInode::Rewind() {
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
    

    bool TempFSInode::Read(uint64_t offset, uint8_t* bytes, uint64_t count) {
        bool isOpen = p_isOpen;
        if (!p_isOpen) {
            if (!Open())
                return false;
        }
        if (!Seek(offset))
            return false;
        bool status = ReadStream(bytes, count);
        if (!isOpen)
            status |= Close();
        return status;
    }
    
    bool TempFSInode::Write(uint64_t offset, const uint8_t* bytes, uint64_t count) {
        bool isOpen = p_isOpen;
        if (!p_isOpen) {
            if (!Open())
                return false;
        }
        if (!Seek(offset))
            return false;
        bool status = WriteStream(bytes, count);
        if (!isOpen)
            status |= Close();
        return status;
    }

    bool TempFSInode::Expand(size_t new_size) {
        MemBlock* mem_block = new MemBlock;
        if (mem_block == nullptr) {
            SetLastError(InodeError::ALLOCATION_FAILED);
            return false;
        }
        mem_block->size = ALIGN_UP((new_size - m_size), p_blockSize);
        mem_block->address = WorldOS::g_KPM->AllocatePages(mem_block->size / PAGE_SIZE);
        if ((mem_block->address) == nullptr) {
            SetLastError(InodeError::ALLOCATION_FAILED);
            return false;
        }
        m_size = new_size;
        m_data.insert(mem_block);
        SetLastError(InodeError::SUCCESS);
        return true;
    }

    bool TempFSInode::AddChild(TempFSInode* child) {
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

    TempFSInode* TempFSInode::GetChild(uint64_t ID) const {
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

    TempFSInode* TempFSInode::GetChild(const char* name) const {
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

    uint64_t TempFSInode::GetChildCount() const {
        return m_children.getCount();
    }

    bool TempFSInode::RemoveChild(TempFSInode* child) {
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
        return m_parent;
    }

    void TempFSInode::ResetID(uint32_t seed) {
        if (seed != 0)
            srand(seed);
        p_ID = ((uint64_t)rand() << 32) | rand();
    }


    void TempFSInode::SetLastError(InodeError error) const {
        p_lastError = error;
    }
}
