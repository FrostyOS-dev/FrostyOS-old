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

#include "TempFileSystem.hpp"

#include <errno.h>
#include <string.h>

#include <Memory/kmalloc.hpp>

#include <Data-structures/Buffer.hpp>

namespace TempFS {
    TempFileSystem::TempFileSystem(size_t blockSize, FilePrivilegeLevel rootPrivilege) : m_rootPrivilege(rootPrivilege) {
        p_blockSize = blockSize;
    }

    TempFileSystem::~TempFileSystem() {

    }

    int TempFileSystem::CreateFile(FilePrivilegeLevel current_privilege, const char* parent, const char* name, size_t size, bool inherit_permissions, FilePrivilegeLevel privilege) {
        if (name == nullptr)
            return -EFAULT;
        TempFSInode* parent_inode = GetInode(parent);
        if (parent_inode == nullptr && parent != nullptr)
            return false;
        FilePrivilegeLevel parent_priv = parent_inode == nullptr ? m_rootPrivilege : parent_inode->GetPrivilegeLevel();
        if (current_privilege.UID == parent_priv.UID || current_privilege.UID == 0) {
            if (!((parent_priv.ACL & ACL_USER_WRITE) > 0))
                return -EACCES;
        }
        else if (current_privilege.GID == parent_priv.GID || current_privilege.GID == 0) {
            if (!((parent_priv.ACL & ACL_GROUP_WRITE) > 0))
                return -EACCES;
        }
        else {
            if (!((parent_priv.ACL & ACL_OTHER_WRITE) > 0))
                return -EACCES;
        }
        TempFSInode* inode = new TempFSInode;
        FilePrivilegeLevel priv = {0, 0, 00644};
        if (inherit_permissions)
            priv = parent_priv;
        else
            priv = privilege;
        int rc = inode->Create(name, parent_inode, InodeType::File, this, priv, p_blockSize);
        if (rc < 0)
            return rc;
        if (size > 0) {
            int rc = inode->Expand(size);
            if (rc < 0)
                return rc;
        }
        return ESUCCESS;
    }

    int TempFileSystem::CreateFile(FilePrivilegeLevel current_privilege, TempFSInode* parent, const char* name, size_t size, bool inherit_permissions, FilePrivilegeLevel privilege) {
        if (name == nullptr)
            return -EFAULT;
        FilePrivilegeLevel parent_priv = parent == nullptr ? m_rootPrivilege : parent->GetPrivilegeLevel();
        if (current_privilege.UID == parent_priv.UID || current_privilege.UID == 0) {
            if (!((parent_priv.ACL & ACL_USER_WRITE) > 0))
                return -EACCES;
        }
        else if (current_privilege.GID == parent_priv.GID || current_privilege.GID == 0) {
            if (!((parent_priv.ACL & ACL_GROUP_WRITE) > 0))
                return -EACCES;
        }
        else {
            if (!((parent_priv.ACL & ACL_OTHER_WRITE) > 0))
                return -EACCES;
        }
        TempFSInode* inode = new TempFSInode;
        FilePrivilegeLevel priv = {0, 0, 00644};
        if (inherit_permissions)
            priv = parent_priv;
        else
            priv = privilege;
        int rc = inode->Create(name, parent, InodeType::File, this, priv, p_blockSize);
        if (rc < 0)
            return rc;
        if (size > 0) {
            int rc = inode->Expand(size);
            if (rc < 0)
                return rc;
        }
        return ESUCCESS;
    }

    int TempFileSystem::CreateFolder(FilePrivilegeLevel current_privilege, const char* parent, const char* name, bool inherit_permissions, FilePrivilegeLevel privilege) {
        if (name == nullptr)
            return -EFAULT;
        TempFSInode* parent_inode = GetInode(parent);
        if (parent_inode == nullptr && parent != nullptr)
            return -ENOENT;
        FilePrivilegeLevel parent_priv = parent_inode == nullptr ? m_rootPrivilege : parent_inode->GetPrivilegeLevel();
        if (current_privilege.UID == parent_priv.UID || current_privilege.UID == 0) {
            if (!((parent_priv.ACL & ACL_USER_WRITE) > 0))
                return -EACCES;
        }
        else if (current_privilege.GID == parent_priv.GID || current_privilege.GID == 0) {
            if (!((parent_priv.ACL & ACL_GROUP_WRITE) > 0))
                return -EACCES;
        }
        else {
            if (!((parent_priv.ACL & ACL_OTHER_WRITE) > 0))
                return -EACCES;
        }
        TempFSInode* inode = new TempFSInode;
        FilePrivilegeLevel priv = {0, 0, 00644};
        if (inherit_permissions)
            priv = parent_priv;
        else
            priv = privilege;
        int rc = inode->Create(name, parent_inode, InodeType::Folder, this, priv, p_blockSize);
        if (rc < 0)
            return rc;
        return ESUCCESS;
    }

    int TempFileSystem::CreateFolder(FilePrivilegeLevel current_privilege, TempFSInode* parent, const char* name, bool inherit_permissions, FilePrivilegeLevel privilege) {
        if (name == nullptr)
            return -EFAULT;
        FilePrivilegeLevel parent_priv = parent == nullptr ? m_rootPrivilege : parent->GetPrivilegeLevel();
        if (current_privilege.UID == parent_priv.UID || current_privilege.UID == 0) {
            if (!((parent_priv.ACL & ACL_USER_WRITE) > 0))
                return -EACCES;
        }
        else if (current_privilege.GID == parent_priv.GID || current_privilege.GID == 0) {
            if (!((parent_priv.ACL & ACL_GROUP_WRITE) > 0))
                return -EACCES;
        }
        else {
            if (!((parent_priv.ACL & ACL_OTHER_WRITE) > 0))
                return -EACCES;
        }
        TempFSInode* inode = new TempFSInode;
        FilePrivilegeLevel priv = {0, 0, 00644};
        if (inherit_permissions)
            priv = parent_priv;
        else
            priv = privilege;
        int rc = inode->Create(name, parent, InodeType::Folder, this, priv, p_blockSize);
        if (rc < 0)
            return rc;
        return ESUCCESS;
    }

    int TempFileSystem::CreateSymLink(FilePrivilegeLevel current_privilege, const char* parent, const char* name, const char* target, bool inherit_permissions, FilePrivilegeLevel privilege) {
        if (name == nullptr)
            return -EFAULT;
        TempFSInode* parent_inode = GetInode(parent);
        if (parent_inode == nullptr && parent != nullptr)
            return -ENOENT;
        TempFSInode* target_inode = GetInode(target);
        if (target_inode == nullptr)
            return -ENOENT;
        FilePrivilegeLevel parent_priv = parent_inode == nullptr ? m_rootPrivilege : parent_inode->GetPrivilegeLevel();
        if (current_privilege.UID == parent_priv.UID || current_privilege.UID == 0) {
            if (!((parent_priv.ACL & ACL_USER_WRITE) > 0))
                return -EACCES;
        }
        else if (current_privilege.GID == parent_priv.GID || current_privilege.GID == 0) {
            if (!((parent_priv.ACL & ACL_GROUP_WRITE) > 0))
                return -EACCES;
        }
        else {
            if (!((parent_priv.ACL & ACL_OTHER_WRITE) > 0))
                return -EACCES;
        }
        TempFSInode* inode = new TempFSInode;
        FilePrivilegeLevel priv = {0, 0, 00644};
        if (inherit_permissions)
            priv = parent_priv;
        else
            priv = privilege;
        int rc = inode->Create(name, parent_inode, InodeType::SymLink, this, priv, p_blockSize, target_inode);
        if (rc < 0)
            return rc;
        return ESUCCESS;
    }

    int TempFileSystem::CreateSymLink(FilePrivilegeLevel current_privilege, TempFSInode* parent, const char* name, TempFSInode* target, bool inherit_permissions, FilePrivilegeLevel privilege) {
        if (name == nullptr)
            return -EFAULT;
        FilePrivilegeLevel parent_priv = parent == nullptr ? m_rootPrivilege : parent->GetPrivilegeLevel();
        if (current_privilege.UID == parent_priv.UID) {
            if (!((parent_priv.ACL & ACL_USER_WRITE) > 0))
                return -EACCES;
        }
        else if (current_privilege.GID == parent_priv.GID) {
            if (!((parent_priv.ACL & ACL_GROUP_WRITE) > 0))
                return -EACCES;
        }
        else {
            if (!((parent_priv.ACL & ACL_OTHER_WRITE) > 0))
                return -EACCES;
        }
        TempFSInode* inode = new TempFSInode;
        FilePrivilegeLevel priv = {0, 0, 00644};
        if (inherit_permissions)
            priv = parent_priv;
        else
            priv = privilege;
        int rc = inode->Create(name, parent, InodeType::SymLink, this, priv, p_blockSize, target);
        if (rc < 0)
            return rc;
        return ESUCCESS;
    }

    int TempFileSystem::DeleteInode(FilePrivilegeLevel current_privilege, const char* path, bool recursive, bool delete_name) {
        if (path == nullptr)
            return -EFAULT;
        int status;
        TempFSInode* inode = GetInode(path, nullptr, nullptr, &status);
        if (inode == nullptr)
            return status;
        TempFSInode* parent = inode->GetParent();
        FilePrivilegeLevel parent_priv = parent == nullptr ? m_rootPrivilege : parent->GetPrivilegeLevel();
        // Deletion is ALWAYS allowed if we are the owner
        if (current_privilege.GID == parent_priv.GID && current_privilege.UID != parent_priv.UID) {
            if (!((parent_priv.ACL & ACL_GROUP_WRITE) > 0))
                return -EACCES;
        }
        else if (current_privilege.UID != parent_priv.UID) {
            if (!((parent_priv.ACL & ACL_OTHER_WRITE) > 0))
                return -EACCES;
        }
        if (inode->GetChildCount() > 0 && !recursive)
            return -ENOTEMPTY;
        int rc = inode->Delete(false, delete_name);
        if (rc < 0)
            return rc;
        delete inode;
        return ESUCCESS;
    }

    int TempFileSystem::DeleteInode(FilePrivilegeLevel current_privilege, TempFSInode* inode, bool recursive, bool delete_name) {
        if (inode == nullptr)
            return -EFAULT;
        TempFSInode* parent = inode->GetParent();
        FilePrivilegeLevel parent_priv = parent == nullptr ? m_rootPrivilege : parent->GetPrivilegeLevel();
        // Deletion is ALWAYS allowed if we are the owner
        if (current_privilege.GID == parent_priv.GID && current_privilege.UID != parent_priv.UID) {
            if (!((parent_priv.ACL & ACL_GROUP_WRITE) > 0))
                return -EACCES;
        }
        else if (current_privilege.UID != parent_priv.UID) {
            if (!((parent_priv.ACL & ACL_OTHER_WRITE) > 0))
                return -EACCES;
        }
        if (inode->GetChildCount() > 0 && !recursive)
            return -ENOTEMPTY;
        int rc = inode->Delete(false, delete_name);
        if (rc < 0)
            return rc;
        delete inode;
        return ESUCCESS;
    }

    int TempFileSystem::DestroyFileSystem() {
        m_rootInodes.lock();
        for (uint64_t i = 0; i < m_rootInodes.getCount(); i++) {
            TempFSInode* inode = m_rootInodes.get(0);
            if (inode != nullptr) {
                inode->Delete();
                delete inode;
            }
        }
        m_rootInodes.unlock();
        return ESUCCESS;
    }

    void TempFileSystem::CreateNewRootInode(TempFSInode* inode) {
        m_rootInodes.lock();
        m_rootInodes.insert(inode);
        m_rootInodes.unlock();
    }

    void TempFileSystem::DeleteRootInode(TempFSInode* inode) {
        m_rootInodes.lock();
        m_rootInodes.remove(inode);
        m_rootInodes.unlock();
    }

    Inode* TempFileSystem::GetRootInode(uint64_t index, int* status) const {
        m_rootInodes.lock();
        if (index >= m_rootInodes.getCount()) {
            m_rootInodes.unlock();
            if (status != nullptr)
                *status = -EINVAL;
            return nullptr;
        }
        Inode* inode = m_rootInodes.get(index);
        m_rootInodes.unlock();
        if (inode == nullptr && status != nullptr)
            *status = -ENOSYS;
        return inode;
    }

    uint64_t TempFileSystem::GetRootInodeCount() const {
        m_rootInodes.lock();
        uint64_t count = m_rootInodes.getCount();
        m_rootInodes.unlock();
        return count;
    }

    TempFSInode* TempFileSystem::GetSubInode(TempFSInode* parent, const char* path, TempFSInode** lastInode, int64_t* end_index, int* status) {
        if (path == nullptr) {
            if (status != nullptr)
                *status = -EFAULT;
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
        TempFSInode* last_inode = parent;
        while (c) {
            if (c == PATH_SEPARATOR) {
                if (last_separator < i && (i - last_separator) > 1) {
                    char* name = (char*)kmalloc(i - (last_separator + 1) + 1);
                    if (name == nullptr) {
                        if (status != nullptr)
                            *status = -ENOMEM;
                        if (lastInode != nullptr)
                            *lastInode = last_inode;
                        if (end_index != nullptr)
                            *end_index = i;
                        return nullptr;
                    }
                    strncpy(name, &(path[last_separator + 1]), i - (last_separator + 1));
                    name[i - (last_separator + 1)] = 0;
                    if (last_inode != nullptr) {
                        if (strcmp(name, "..") == 0)
                            last_inode = last_inode->GetParent();
                        else if (strcmp(name, ".") != 0) {
                            int i_status;
                            TempFSInode* inode = last_inode->GetTMPFSChild(name, &i_status);
                            if (inode == nullptr) {
                                kfree(name);
                                if (status != nullptr)
                                    *status = i_status;
                                if (lastInode != nullptr)
                                    *lastInode = last_inode;
                                if (end_index != nullptr)
                                    *end_index = last_separator;
                                return nullptr;
                            }
                            last_inode = inode;
                        }
                    }
                    else {
                        if (strcmp(name, ".") != 0) {
                            if (strcmp(name, "..") == 0) {
                                // The VFS needs to be notified that we are going up a directory outside the fs.
                                // We do this by setting the last inode to nullptr and returning nullptr, while the error is success.
                                // This is a hack, but it is faster than the VFS pre-scanning the path.
                                kfree(name);
                                if (status != nullptr)
                                    *status = ESUCCESS;
                                if (lastInode != nullptr)
                                    *lastInode = nullptr;
                                if (end_index != nullptr)
                                    *end_index = i;
                                return nullptr;
                            }
                            else {
                                m_rootInodes.lock();
                                for (uint64_t j = 0; j < m_rootInodes.getCount(); j++) {
                                    TempFSInode* inode = m_rootInodes.get(j);
                                    if (inode == nullptr) {
                                        kfree(name);
                                        m_rootInodes.unlock();
                                        if (status != nullptr)
                                            *status = -ENOSYS;
                                        if (lastInode != nullptr)
                                            *lastInode = last_inode;
                                        if (end_index != nullptr)
                                            *end_index = last_separator;
                                        return nullptr;
                                    }
                                    if (strcmp(name, inode->GetName()) == 0) {
                                        last_inode = inode;
                                        m_rootInodes.unlock();
                                        break;
                                    }
                                }
                                m_rootInodes.unlock();
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
                if (status != nullptr)
                    *status = -ENOMEM;
                if (lastInode != nullptr)
                    *lastInode = last_inode;
                if (end_index != nullptr)
                    *end_index = last_separator;
                return nullptr;
            }
            strcpy(name, &(path[last_separator + 1]));
            name[i - (last_separator + 1)] = '\0';
            if (last_inode != nullptr) {
                int i_status;
                TempFSInode* inode = last_inode->GetTMPFSChild(name, &i_status);
                if (inode == nullptr) {
                    kfree(name);
                    if (status != nullptr)
                        *status = i_status;
                    if (lastInode != nullptr)
                        *lastInode = last_inode;
                    if (end_index != nullptr)
                        *end_index = last_separator;
                    return nullptr;
                }
                last_inode = inode;
            }
            else {
                for (uint64_t j = 0; j < m_rootInodes.getCount(); j++) {
                    TempFSInode* inode = m_rootInodes.get(j);
                    if (inode == nullptr) {
                        kfree(name);
                        if (status != nullptr)
                            *status = -ENOSYS;
                        if (lastInode != nullptr)
                            *lastInode = last_inode;
                        if (end_index != nullptr)
                            *end_index = last_separator;
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
        if (last_inode == nullptr && status != nullptr)
            *status = -ENOENT;
        if (lastInode != nullptr)
            *lastInode = last_inode;
        if (end_index != nullptr)
            *end_index = i;
        return last_inode;
    }

    TempFSInode* TempFileSystem::GetInode(const char* path, TempFSInode** lastInode, int64_t* end_index, int* status) {
        return GetSubInode(nullptr, path, lastInode, end_index, status);
    }

    FileSystemType TempFileSystem::GetType() const {
        return FileSystemType::TMPFS;
    }

    FilePrivilegeLevel TempFileSystem::GetRootPrivilege() const {
        return m_rootPrivilege;
    }
}