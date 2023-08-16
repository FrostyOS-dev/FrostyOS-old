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

#include "TempFileSystem.hpp"

#include <string.h>

#include <Memory/kmalloc.hpp>

namespace TempFS {
    TempFileSystem::TempFileSystem(size_t blockSize) {
        p_blockSize = blockSize;
        p_lastError = FileSystemError::SUCCESS;
    }

    TempFileSystem::~TempFileSystem() {

    }

    bool TempFileSystem::CreateFile(const char* parent, const char* name, size_t size) {
        if (name == nullptr) {
            SetLastError(FileSystemError::INVALID_ARGUMENTS);
            return false;
        }
        TempFSInode* parent_inode = GetInode(parent);
        if (parent_inode == nullptr && parent != nullptr)
            return false;
        TempFSInode* inode = new TempFSInode;
        if (!inode->Create(name, parent_inode, InodeType::File, this, p_blockSize)) {
            if (inode->GetLastError() == InodeError::INVALID_TYPE)
                SetLastError(FileSystemError::INVALID_ARGUMENTS);
            else
                SetLastError(FileSystemError::INTERNAL_ERROR);
            return false;
        }
        if (size > 0) {
            if (!inode->Expand(size)) {
                if (inode->GetLastError() == InodeError::ALLOCATION_FAILED)
                    SetLastError(FileSystemError::ALLOCATION_FAILED);
                else
                    SetLastError(FileSystemError::INTERNAL_ERROR);
                return false;
            }
        }
        SetLastError(FileSystemError::SUCCESS);
        return true;
    }

    bool TempFileSystem::CreateFile(TempFSInode* parent, const char* name, size_t size) {
        if (name == nullptr) {
            SetLastError(FileSystemError::INVALID_ARGUMENTS);
            return false;
        }
        TempFSInode* inode = new TempFSInode;
        if (!inode->Create(name, parent, InodeType::File, this, p_blockSize)) {
            if (inode->GetLastError() == InodeError::INVALID_TYPE)
                SetLastError(FileSystemError::INVALID_ARGUMENTS);
            else
                SetLastError(FileSystemError::INTERNAL_ERROR);
            return false;
        }
        if (size > 0) {
            if (!inode->Expand(size)) {
                if (inode->GetLastError() == InodeError::ALLOCATION_FAILED)
                    SetLastError(FileSystemError::ALLOCATION_FAILED);
                else
                    SetLastError(FileSystemError::INTERNAL_ERROR);
                return false;
            }
        }
        SetLastError(FileSystemError::SUCCESS);
        return true;
    }

    bool TempFileSystem::CreateFolder(const char* parent, const char* name) {
        if (name == nullptr) {
            SetLastError(FileSystemError::INVALID_ARGUMENTS);
            return false;
        }
        TempFSInode* parent_inode = GetInode(parent);
        if (parent_inode == nullptr && parent != nullptr)
            return false;
        TempFSInode* inode = new TempFSInode;
        if (!inode->Create(name, parent_inode, InodeType::Folder, this, p_blockSize)) {
            if (inode->GetLastError() == InodeError::INVALID_TYPE)
                SetLastError(FileSystemError::INVALID_ARGUMENTS);
            else
                SetLastError(FileSystemError::INTERNAL_ERROR);
            return false;
        }
        SetLastError(FileSystemError::SUCCESS);
        return true;
    }

    bool TempFileSystem::CreateFolder(TempFSInode* parent, const char* name) {
        if (name == nullptr) {
            SetLastError(FileSystemError::INVALID_ARGUMENTS);
            return false;
        }
        TempFSInode* inode = new TempFSInode;
        if (!inode->Create(name, parent, InodeType::Folder, this, p_blockSize)) {
            if (inode->GetLastError() == InodeError::INVALID_TYPE)
                SetLastError(FileSystemError::INVALID_ARGUMENTS);
            else
                SetLastError(FileSystemError::INTERNAL_ERROR);
            return false;
        }
        SetLastError(FileSystemError::SUCCESS);
        return true;
    }

    bool TempFileSystem::CreateSymLink(const char* parent, const char* name, const char* target) {
        if (name == nullptr) {
            SetLastError(FileSystemError::INVALID_ARGUMENTS);
            return false;
        }
        TempFSInode* parent_inode = GetInode(parent);
        if (parent_inode == nullptr && parent != nullptr)
            return false;
        TempFSInode* target_inode = GetInode(target);
        if (target_inode == nullptr)
            return false;
        TempFSInode* inode = new TempFSInode;
        if (!inode->Create(name, parent_inode, InodeType::SymLink, this, p_blockSize, target_inode)) {
            if (inode->GetLastError() == InodeError::INVALID_TYPE)
                SetLastError(FileSystemError::INVALID_ARGUMENTS);
            else
                SetLastError(FileSystemError::INTERNAL_ERROR);
            return false;
        }
        SetLastError(FileSystemError::SUCCESS);
        return true;
    }

    bool TempFileSystem::CreateSymLink(TempFSInode* parent, const char* name, TempFSInode* target) {
        if (name == nullptr) {
            SetLastError(FileSystemError::INVALID_ARGUMENTS);
            return false;
        }
        TempFSInode* inode = new TempFSInode;
        if (!inode->Create(name, parent, InodeType::SymLink, this, p_blockSize, target)) {
            if (inode->GetLastError() == InodeError::INVALID_TYPE)
                SetLastError(FileSystemError::INVALID_ARGUMENTS);
            else
                SetLastError(FileSystemError::INTERNAL_ERROR);
            return false;
        }
        SetLastError(FileSystemError::SUCCESS);
        return true;
    }

    bool TempFileSystem::DeleteInode(const char* path, bool recursive) {
        if (path == nullptr) {
            SetLastError(FileSystemError::INVALID_ARGUMENTS);
            return false;
        }
        TempFSInode* inode = GetInode(path);
        if (inode == nullptr)
            return false; // Error code is already set
        if (inode->GetChildCount() > 0 && !recursive) {
            SetLastError(FileSystemError::RECURSION_ERROR);
            return false;
        }
        if (!inode->Delete()) {
            SetLastError(FileSystemError::INTERNAL_ERROR);
            return false;
        }
        delete inode;
        SetLastError(FileSystemError::SUCCESS);
        return true;
    }

    bool TempFileSystem::DeleteInode(TempFSInode* inode, bool recursive) {
        if (inode == nullptr) {
            SetLastError(FileSystemError::INVALID_ARGUMENTS);
            return false;
        }
        if (inode->GetChildCount() > 0 && !recursive) {
            SetLastError(FileSystemError::RECURSION_ERROR);
            return false;
        }
        if (!inode->Delete()) {
            SetLastError(FileSystemError::INTERNAL_ERROR);
            return false;
        }
        delete inode;
        SetLastError(FileSystemError::SUCCESS);
        return true;
    }

    void TempFileSystem::CreateNewRootInode(TempFSInode* inode) {
        m_rootInodes.insert(inode);
    }

    void TempFileSystem::DeleteRootInode(TempFSInode* inode) {
        m_rootInodes.remove(inode);
    }


    TempFSInode* TempFileSystem::GetInode(const char* path, TempFSInode** lastInode, int64_t* end_index) {
        if (path == nullptr) {
            SetLastError(FileSystemError::INVALID_ARGUMENTS);
            if (lastInode != nullptr)
                *lastInode = nullptr;
            return nullptr;
        }
        int64_t i = 0;
        int64_t last_separator = -1;
        if (path[i] == PATH_SEPARATOR) {
            last_separator = 0;
            i++;
        }
        char c = path[i];
        TempFSInode* last_inode = nullptr;
        while (c) {
            if (c == PATH_SEPARATOR) {
                if (last_separator < i && (i - last_separator) > 1) {
                    char* name = (char*)kmalloc(i - (last_separator + 1) + 1);
                    if (name == nullptr) {
                        SetLastError(FileSystemError::ALLOCATION_FAILED);
                        if (lastInode != nullptr)
                            *lastInode = last_inode;
                        if (end_index != nullptr)
                            *end_index = i;
                        return nullptr;
                    }
                    strncpy(name, &(path[last_separator + 1]), i - (last_separator + 1));
                    name[i - (last_separator + 1)] = 0;
                    if (last_inode != nullptr) {
                        TempFSInode* inode = last_inode->GetChild(name);
                        if (inode == nullptr) {
                            kfree(name);
                            InodeError error = last_inode->GetLastError();
                            if (error == InodeError::INVALID_ARGUMENTS || error == InodeError::INVALID_TYPE)
                                SetLastError(FileSystemError::INVALID_ARGUMENTS);
                            else
                                SetLastError(FileSystemError::INTERNAL_ERROR);
                            if (lastInode != nullptr)
                                *lastInode = last_inode;
                            if (end_index != nullptr)
                                *end_index = i;
                            return nullptr;
                        }
                        last_inode = inode;
                    }
                    else {
                        for (uint64_t j = 0; j < m_rootInodes.getCount(); j++) {
                            TempFSInode* inode = m_rootInodes.get(j);
                            if (inode == nullptr) {
                                SetLastError(FileSystemError::INTERNAL_ERROR);
                                if (lastInode != nullptr)
                                    *lastInode = last_inode;
                                if (end_index != nullptr)
                                    *end_index = i;
                                return nullptr;
                            }
                            if (strcmp(name, inode->GetName()) == 0) {
                                last_inode = inode;
                                break;
                            }
                        }
                    }
                    kfree(name);
                }
                last_separator = i;
            }
            i++;
            c = path[i];
        }
        if (last_separator < i && (i - last_separator) > 1) {
            char* name = (char*)kmalloc(i - (last_separator + 1) + 1);
            if (name == nullptr) {
                SetLastError(FileSystemError::ALLOCATION_FAILED);
                if (lastInode != nullptr)
                    *lastInode = last_inode;
                if (end_index != nullptr)
                    *end_index = i;
                return nullptr;
            }
            strcpy(name, &(path[last_separator + 1]));
            name[i - (last_separator + 1)] = '\0';
            if (last_inode != nullptr) {
                TempFSInode* inode = last_inode->GetChild(name);
                if (inode == nullptr) {
                    kfree(name);
                    InodeError error = last_inode->GetLastError();
                    if (error == InodeError::INVALID_ARGUMENTS || error == InodeError::INVALID_TYPE)
                        SetLastError(FileSystemError::INVALID_ARGUMENTS);
                    else
                        SetLastError(FileSystemError::INTERNAL_ERROR);
                    if (lastInode != nullptr)
                        *lastInode = last_inode;
                    if (end_index != nullptr)
                        *end_index = i;
                    return nullptr;
                }
                last_inode = inode;
            }
            else {
                for (uint64_t j = 0; j < m_rootInodes.getCount(); j++) {
                    TempFSInode* inode = m_rootInodes.get(j);
                    if (inode == nullptr) {
                        kfree(name);
                        SetLastError(FileSystemError::INTERNAL_ERROR);
                        if (lastInode != nullptr)
                            *lastInode = last_inode;
                        if (end_index != nullptr)
                            *end_index = i;
                        return nullptr;
                    }
                    if (strcmp(name, inode->GetName()) == 0) {
                        last_inode = inode;
                        break;
                    }
                }
                kfree(name);
            }
        }
        if (last_inode == nullptr) {
            SetLastError(FileSystemError::INVALID_ARGUMENTS);
        }
        else
            SetLastError(FileSystemError::SUCCESS);
        if (lastInode != nullptr)
            *lastInode = last_inode;
        if (end_index != nullptr)
            *end_index = i;
        return last_inode;
    }
}