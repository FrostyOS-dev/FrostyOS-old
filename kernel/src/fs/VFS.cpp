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

#include "VFS.hpp"
#include "FileStream.hpp"

#include "TempFS/TempFileSystem.hpp"

#include <Memory/kmalloc.hpp>

#include <cstdio>
#include <errno.h>
#include <string.h>

VFS* g_VFS = nullptr;

VFS::VFS() : m_root(nullptr) {

}

VFS::VFS(FileSystemType root_type) : m_root(nullptr) {
    MountRoot(root_type);
}

VFS::~VFS() {

}

int VFS::MountRoot(FileSystemType type) {
    m_root = new VFS_MountPoint;
    m_root->type = type;
    m_root->RootInode = nullptr;
    m_root->parent = nullptr;
    switch (type) {
        case FileSystemType::TMPFS:
            m_root->fs = (FileSystem*)(new TempFS::TempFileSystem(PAGE_SIZE, {0, 0, 00755}));
            break;
        default:
            delete m_root;
            return -ENOSYS;
    }
    m_mountPoints.lock();
    m_mountPoints.insert(m_root);
    m_mountPoints.unlock();
    return ESUCCESS;
}

int VFS::Mount(FilePrivilegeLevel current_privilege, const char* path, FileSystemType type) {
    if (current_privilege.UID != 0) {
        return -EACCES;
    }
    Inode* inode = nullptr;
    VFS_MountPoint* parent_mountPoint = GetMountPoint(path, nullptr, &inode);
    if (parent_mountPoint == nullptr || inode == nullptr) {
        return -EINVAL;
    }
    if (inode->GetType() != InodeType::Folder) {
        return -ENOTDIR;
    }
    if (inode->GetChildCount() > 0) {
        return -ENOTEMPTY;
    }
    VFS_MountPoint* mountPoint = new VFS_MountPoint;
    mountPoint->type = type;
    mountPoint->RootInode = inode;
    FileSystem* fs = nullptr;
    switch (type) {
        case FileSystemType::TMPFS:
            fs = (FileSystem*)(new TempFS::TempFileSystem(PAGE_SIZE, inode->GetPrivilegeLevel()));
            break;
        default:
            delete mountPoint;
            return -ENOSYS;
    }
    mountPoint->fs = fs;
    mountPoint->parent = parent_mountPoint;
    m_mountPoints.lock();
    m_mountPoints.insert(mountPoint);
    m_mountPoints.unlock();
    return ESUCCESS;
}

int VFS::Unmount(FilePrivilegeLevel current_privilege, const char* path, VFS_WorkingDirectory* working_directory) {
    if (current_privilege.UID != 0) {
        return -EACCES;
    }
    Inode* inode = nullptr;
    VFS_MountPoint* parent_mountPoint = GetMountPoint(path, working_directory, &inode);
    if (parent_mountPoint == nullptr || inode == nullptr) {
        return -EINVAL;
    }

    VFS_MountPoint* mountPoint = nullptr;
    m_mountPoints.lock();
    for (uint64_t i = 0; i < m_mountPoints.getCount(); i++) {
        VFS_MountPoint* i_mountPoint = m_mountPoints.get(i);
        if (i_mountPoint == nullptr) {
            m_mountPoints.unlock();
            return -ENOSYS;
        }
        if (i_mountPoint->RootInode == inode) {
            mountPoint = i_mountPoint;
            break;
        }
    }

    // we need to check if this mountpoint has any sub-mountpoints
    for (uint64_t i = 0; i < m_mountPoints.getCount(); i++) {
        VFS_MountPoint* i_mountPoint = m_mountPoints.get(i);
        if (i_mountPoint == nullptr) {
            m_mountPoints.unlock();
            return -ENOSYS;
        }
        if (i_mountPoint->parent == mountPoint) {
            m_mountPoints.unlock();
            return -EBUSY;
        }
    }
    m_mountPoints.unlock();

    // close any streams
    m_streams.lock();
    for (uint64_t i = 0; i < m_streams.getCount(); i++) {
        FileStream* i_stream = m_streams.get(i);
        if (i_stream == nullptr) {
            m_streams.unlock();
            return -ENOSYS;
        }
        if (i_stream->GetMountPoint() == mountPoint) {
            m_streams.remove(i);
            m_streams.unlock();
            (void)(i_stream->Close()); // ignore return value
            delete i_stream;
        }
    }
    m_streams.unlock();

    // close any directory streams
    m_directoryStreams.lock();
    for (uint64_t i = 0; i < m_directoryStreams.getCount(); i++) {
        DirectoryStream* i_stream = m_directoryStreams.get(i);
        if (i_stream == nullptr) {
            m_directoryStreams.unlock();
            return -ENOSYS;
        }
        if (i_stream->GetFileSystem() == mountPoint->fs) {
            m_directoryStreams.remove(i);
            m_directoryStreams.unlock();
            (void)(i_stream->Close()); // ignore return value
            delete i_stream;
        }
    }
    m_directoryStreams.unlock();

    m_mountPoints.lock();
    m_mountPoints.remove(mountPoint);
    m_mountPoints.unlock();
    mountPoint->fs->DestroyFileSystem();
    delete mountPoint->fs;
    delete mountPoint;
    
    return ESUCCESS;
}

int VFS::CreateFile(FilePrivilegeLevel current_privilege, const char* parent, const char* name, size_t size, bool inherit_permissions, FilePrivilegeLevel privilege) {
    Inode* parent_inode = nullptr;
    int rc = 0;
    VFS_MountPoint* mountPoint = GetChildMountPoint(parent, nullptr, &parent_inode, &rc);
    if (mountPoint == nullptr || (parent_inode == nullptr && rc != ESUCCESS) || name == nullptr || (parent_inode != nullptr && parent_inode->GetType() != InodeType::Folder)) {
        return -EINVAL;
    }
    switch (mountPoint->type) {
        case FileSystemType::TMPFS:
            if (mountPoint->fs == nullptr) {
                return -ENOSYS;
            }
            {
                using namespace TempFS;
                TempFileSystem* fs = (TempFileSystem*)mountPoint->fs;
                uint64_t name_length = strlen(name);
                char* i_name = new char[name_length + 1];
                memcpy(i_name, name, name_length);
                i_name[name_length] = '\0';
                return fs->CreateFile(current_privilege, (TempFSInode*)parent_inode, i_name, size, inherit_permissions, privilege);
            }
            break;
        default:
            return -ENOSYS;
            break;
    }
    return -ENOSYS;
}

int VFS::CreateFolder(FilePrivilegeLevel current_privilege, const char* parent, const char* name, bool inherit_permissions, FilePrivilegeLevel privilege) {
    Inode* parent_inode = nullptr;
    int rc = 0;
    VFS_MountPoint* mountPoint = GetChildMountPoint(parent, nullptr, &parent_inode, &rc);
    if (mountPoint == nullptr || (parent_inode == nullptr && rc != ESUCCESS) || name == nullptr || (parent_inode != nullptr && parent_inode->GetType() != InodeType::Folder)) {
        return -EINVAL;
    }
    uint64_t name_length = strlen(name);
    char const* i_name = name;
    if (name[name_length - 1] == PATH_SEPARATOR) {
        char* temp_name = new char[name_length];
        memcpy(temp_name, name, name_length - 1);
        temp_name[name_length - 1] = '\0';
        i_name = temp_name;
    }
    else {
        char* temp_name = new char[name_length + 1];
        memcpy(temp_name, name, name_length);
        temp_name[name_length] = '\0';
        i_name = temp_name;
    }
    switch (mountPoint->type) {
        case FileSystemType::TMPFS:
            if (mountPoint->fs == nullptr) {
                return -ENOSYS;
            }
            {
                using namespace TempFS;
                TempFileSystem* fs = (TempFileSystem*)mountPoint->fs;
                return fs->CreateFolder(current_privilege, (TempFSInode*)parent_inode, i_name, inherit_permissions, privilege);
            }
            break;
        default:
            return -ENOSYS;
            break;
    }
    return -ENOSYS;
}

int VFS::CreateSymLink(FilePrivilegeLevel current_privilege, const char* parent, const char* name, const char* target, bool inherit_permissions, FilePrivilegeLevel privilege) {
    Inode* parent_inode = nullptr;
    int rc = 0;
    VFS_MountPoint* mountPoint = GetChildMountPoint(parent, nullptr, &parent_inode, &rc);
    Inode* target_inode = nullptr;
    VFS_MountPoint* target_mountPoint = GetMountPoint(target, nullptr, &target_inode); // currently, parent and target must be on the same mount-point
    if (mountPoint == nullptr || mountPoint != target_mountPoint || (parent_inode == nullptr && rc != ESUCCESS) || name == nullptr || (parent_inode != nullptr && parent_inode->GetType() != InodeType::Folder) || target_inode == nullptr) {
        return -EINVAL;
    }
    uint64_t name_length = strlen(name);
    char const* i_name = name;
    if (name[name_length - 1] == PATH_SEPARATOR) {
        char* temp_name = new char[name_length];
        memcpy(temp_name, name, name_length - 1);
        temp_name[name_length - 1] = '\0';
        i_name = temp_name;
    }
    else {
        char* temp_name = new char[name_length + 1];
        memcpy(temp_name, name, name_length);
        temp_name[name_length] = '\0';
        i_name = temp_name;
    }
    switch (mountPoint->type) {
        case FileSystemType::TMPFS:
            if (mountPoint->fs == nullptr) {
                return -ENOSYS;
            }
            {
                using namespace TempFS;
                TempFileSystem* fs = (TempFileSystem*)mountPoint->fs;
                return fs->CreateSymLink(current_privilege, (TempFSInode*)parent_inode, i_name, (TempFSInode*)target_inode, inherit_permissions, privilege);
            }
            break;
        default:
            return -ENOSYS;
            break;
    }
    return -ENOSYS;
}


int VFS::DeleteInode(FilePrivilegeLevel current_privilege, const char* path, bool recursive, bool delete_name) {
    (void)delete_name;
    Inode* parent_inode = nullptr;
    VFS_MountPoint* mountPoint = GetMountPoint(path, nullptr, &parent_inode);
    if (mountPoint == nullptr || parent_inode == nullptr) {
        return -EINVAL;
    }
    switch (mountPoint->type) {
        case FileSystemType::TMPFS:
            if (mountPoint->fs == nullptr) {
                return -ENOSYS;
            }
            {
                using namespace TempFS;
                TempFileSystem* fs = (TempFileSystem*)mountPoint->fs;
                return fs->DeleteInode(current_privilege, (TempFSInode*)parent_inode, recursive, true);
            }
            break;
        default:
            return -ENOSYS;
            break;
    }
    return -ENOSYS;
}


FileStream* VFS::OpenStream(FilePrivilegeLevel current_privilege, const char* path, uint8_t modes, VFS_WorkingDirectory* working_directory, int* status) {
    Inode* inode = nullptr;
    VFS_MountPoint* mountPoint = GetMountPoint(path, working_directory, &inode);
    if (mountPoint == nullptr || inode == nullptr) {
        if (status != nullptr)
            *status = -EINVAL;
        return nullptr;
    }
    switch (mountPoint->type) {
        case FileSystemType::TMPFS:
            if (mountPoint->fs == nullptr) {
                if (status != nullptr)
                    *status = -ENOSYS;
                return nullptr;
            }
            {
                using namespace TempFS;
                FileStream* stream = new FileStream(inode, mountPoint, modes, current_privilege);
                int rc = stream->Open();
                if (rc != ESUCCESS) {
                    if (status != nullptr)
                        *status = rc;
                    return nullptr;
                }
                m_streams.lock();
                m_streams.insert(stream);
                m_streams.unlock();
                return stream;
            }
            break;
        default:
            if (status != nullptr)
                *status = -ENOSYS;
            return nullptr;
            break;
    }
    if (status != nullptr)
        *status = -ENOSYS;
    return nullptr;
}

int VFS::CloseStream(FileStream* stream) {
    if (stream == nullptr)
        return -EINVAL;
    m_streams.lock();
    for (uint64_t i = 0; i < m_streams.getCount(); i++) {
        FileStream* i_stream = m_streams.get(i);
        if (i_stream == nullptr) {
            m_streams.unlock();
            return -ENOSYS;
        }
        if (stream == i_stream) {
            m_streams.remove(i);
            m_streams.unlock();
            (void)(stream->Close()); // ignore return value
            delete stream;
            return ESUCCESS;
        }
    }
    m_streams.unlock();
    return -EINVAL;
}

DirectoryStream* VFS::OpenDirectoryStream(FilePrivilegeLevel current_privilege, const char* path, uint8_t modes, VFS_WorkingDirectory* working_directory, int* status) {
    if (modes != VFS_READ) {
        if (status != nullptr)
            *status = -EINVAL;
        return nullptr;
    }
    Inode* inode = nullptr;
    VFS_MountPoint* mountPoint = GetMountPoint(path, working_directory, &inode);
    if (mountPoint == nullptr) {
        if (status != nullptr)
            *status = -EINVAL;
        return nullptr;
    }
    DirectoryStream* stream = new DirectoryStream(isMountpoint(path, strlen(path), working_directory) ? nullptr : inode, current_privilege, mountPoint->type, mountPoint->fs);
    int rc = stream->Open();
    if (rc != ESUCCESS) {
        if (status != nullptr)
            *status = rc;
        return nullptr;
    }
    m_directoryStreams.lock();
    m_directoryStreams.insert(stream);
    m_directoryStreams.unlock();
    if (status != nullptr)
        *status = ESUCCESS;
    return stream;
}

int VFS::CloseDirectoryStream(DirectoryStream* stream) {
    if (stream == nullptr)
        return -EINVAL;
    m_directoryStreams.lock();
    for (uint64_t i = 0; i < m_directoryStreams.getCount(); i++) {
        DirectoryStream* i_stream = m_directoryStreams.get(i);
        if (i_stream == nullptr) {
            m_directoryStreams.unlock();
            return -ENOSYS;
        }
        if (stream == i_stream) {
            m_directoryStreams.remove(i);
            m_directoryStreams.unlock();
            (void)(stream->Close()); // ignore return value
            delete stream;
            return ESUCCESS;
        }
    }
    m_directoryStreams.unlock();
    return -EINVAL;
}

bool VFS::IsValidPath(const char* path, VFS_WorkingDirectory* working_directory, int* status) const {
    Inode* inode = nullptr;
    int rc = 0;
    VFS_MountPoint* mountPoint = GetMountPoint(path, working_directory, &inode, &rc);
    if (mountPoint == nullptr || (inode == nullptr && rc != ESUCCESS)) {
        if (status != nullptr)
            *status = -EINVAL;
        return false;
    }
    if (status != nullptr)
        *status = ESUCCESS;
    return true;
}

Inode* VFS::GetInode(const char* path, VFS_WorkingDirectory* working_directory, FileSystem** fs, VFS_MountPoint** mountpoint, int* status) const {
    Inode* inode = nullptr;
    VFS_MountPoint* mountPoint = GetChildMountPoint(path, working_directory, &inode);
    if (mountPoint == nullptr) {
        if (fs != nullptr)
            *fs = nullptr;
        if (mountpoint != nullptr)
            *mountpoint = nullptr;
        if (status != nullptr)
            *status = -EINVAL;
        return nullptr;
    }
    if (fs != nullptr)
        *fs = mountPoint->fs;
    if (mountpoint != nullptr)
        *mountpoint = mountPoint;
    if (status != nullptr)
        *status = ESUCCESS;
    return inode;
}

VFS_MountPoint* VFS::GetMountPoint(FileSystem* fs, int* status) const {
    if (fs == nullptr) {
        if (status != nullptr)
            *status = -EINVAL;
        return nullptr;
    }
    m_mountPoints.lock();
    for (uint64_t i = 0; i < m_mountPoints.getCount(); i++) {
        VFS_MountPoint* mountPoint = m_mountPoints.get(i);
        if (mountPoint == nullptr) {
            m_mountPoints.unlock();
            if (status != nullptr)
                *status = -ENOSYS;
            return nullptr;
        }
        if (mountPoint->fs == fs) {
            m_mountPoints.unlock();
            if (status != nullptr)
                *status = ESUCCESS;
            return mountPoint;
        }
    }
    m_mountPoints.unlock();
    if (status != nullptr)
        *status = -EINVAL;
    return nullptr;
}

VFS_WorkingDirectory* VFS::GetRootWorkingDirectory() const {
    VFS_WorkingDirectory* working_directory = new VFS_WorkingDirectory;
    working_directory->mountpoint = m_root;
    working_directory->inode = nullptr;
    return working_directory;
}

/* Private functions */

VFS_MountPoint* VFS::GetMountPoint(const char* path, VFS_WorkingDirectory* working_directory, Inode** inode, int* status) const {
    if (path == nullptr) {
        if (inode != nullptr)
            *inode = nullptr;
        if (status != nullptr)
            *status = -EINVAL;
        return nullptr;
    }
    if (strcmp(path, "/") == 0) {
        if (inode != nullptr)
            *inode = nullptr;
        if (status != nullptr)
            *status = ESUCCESS;
        return m_root;
    }
    VFS_MountPoint* mountPoint = m_root;
    Inode* last_inode = nullptr;
    int64_t split_index = 0;
    if (path[0] != PATH_SEPARATOR) {
        if (working_directory == nullptr)
            mountPoint = m_root;
        else
            mountPoint = working_directory->mountpoint;
        if (mountPoint == nullptr) {
            if (inode != nullptr)
                *inode = nullptr;
            if (status != nullptr)
                *status = -ENOSYS;
            return nullptr;
        }
        if (strcmp(path, ".") == 0) {
            if (inode != nullptr)
                *inode = working_directory == nullptr ? nullptr : working_directory->inode;
            if (status != nullptr)
                *status = ESUCCESS;
            return mountPoint;
        }
        if (strcmp(path, "..") == 0) {
            if (working_directory == nullptr || working_directory->inode == nullptr) {
                if (inode != nullptr)
                    *inode = nullptr;
                if (status != nullptr)
                    *status = -EINVAL;
                return nullptr;
            }
            else {
                Inode* parent_inode = working_directory->inode->GetParent();
                if (parent_inode == nullptr) {
                    TODO(); // FIXME: why is this here?
                }
                else {
                    if (inode != nullptr)
                        *inode = parent_inode;
                    if (status != nullptr)
                        *status = ESUCCESS;
                    return mountPoint;
                }
            }
        }
        switch (mountPoint->type) {
        case FileSystemType::TMPFS:
            if (mountPoint->fs == nullptr) {
                if (inode != nullptr)
                    *inode = nullptr;
                if (status != nullptr)
                    *status = -ENOSYS;
                return nullptr;
            }
            {
                using namespace TempFS;
                TempFileSystem* fs = (TempFileSystem*)mountPoint->fs;
                TempFSInode* root_inode = working_directory == nullptr ? nullptr : (TempFSInode*)(working_directory->inode);
                int rc = 0;
                TempFSInode* i_inode = fs->GetSubInode(root_inode, path, (TempFSInode**)&last_inode, &split_index, &rc);
                if (i_inode == last_inode && i_inode != nullptr && i_inode != root_inode) {
                    if (inode != nullptr)
                        *inode = i_inode;
                    if (status != nullptr)
                        *status = ESUCCESS;
                    return mountPoint;
                }
                if (i_inode == nullptr && last_inode == nullptr) {
                    if (rc == ESUCCESS) { // This is TempFS signalling to us that we need to go up a directory outside of the mountpoint
                        last_inode = root_inode;
                    }
                    else {
                        if (inode != nullptr)
                            *inode = nullptr;
                        if (status != nullptr)
                            *status = rc;
                        return nullptr;
                    }
                }
            }
            break;
        default:
            if (inode != nullptr)
                *inode = nullptr;
            if (status != nullptr)
                *status = -ENOSYS;
            return nullptr;
        }
        VFS_MountPoint* last_mountPoint = mountPoint;
        m_mountPoints.lock();
        for (uint64_t i = 0; i < m_mountPoints.getCount(); i++) {
            VFS_MountPoint* i_mountPoint = m_mountPoints.get(i);
            if (i_mountPoint == nullptr) {
                m_mountPoints.unlock();
                if (inode != nullptr)
                    *inode = nullptr;
                if (status != nullptr)
                    *status = -ENOSYS;
                return nullptr;
            }
            if (i_mountPoint->RootInode == last_inode) {
                mountPoint = i_mountPoint;
                break;
            }
        }
        m_mountPoints.unlock();
        if (mountPoint == last_mountPoint) {
            if (inode != nullptr)
                *inode = nullptr;
            if (status != nullptr)
                *status = -EINVAL;
            return nullptr;
        }
    }
    while (true) {
        switch (mountPoint->type) {
        case FileSystemType::TMPFS:
            if (mountPoint->fs == nullptr) {
                if (inode != nullptr)
                    *inode = nullptr;
                if (status != nullptr)
                    *status = -ENOSYS;
                return nullptr;
            }
            {
                using namespace TempFS;
                TempFileSystem* fs = (TempFileSystem*)mountPoint->fs;
                TempFSInode* lastInode = (TempFSInode*)(last_inode);
                int rc = 0;
                TempFSInode* i_inode = fs->GetInode(&(path[split_index]), (TempFSInode**)&last_inode, &split_index, &rc);
                if (i_inode == last_inode && i_inode != nullptr) {
                    if (inode != nullptr)
                        *inode = i_inode;
                    if (status != nullptr)
                        *status = ESUCCESS;
                    return mountPoint;
                }
                if (i_inode == nullptr && last_inode == nullptr) {
                    if (rc == ESUCCESS) // This is TempFS signalling to us that we need to go up a directory outside of the mountpoint
                        last_inode = lastInode;
                    else {
                        if (inode != nullptr)
                            *inode = nullptr;
                        if (status != nullptr)
                            *status = rc;
                        return nullptr;
                    }
                }
            }
            break;
        default:
            if (inode != nullptr)
                *inode = nullptr;
            if (status != nullptr)
                *status = -ENOSYS;
            return nullptr;
        }
        VFS_MountPoint* last_mountPoint = mountPoint;
        m_mountPoints.lock();
        for (uint64_t i = 0; i < m_mountPoints.getCount(); i++) {
            VFS_MountPoint* i_mountPoint = m_mountPoints.get(i);
            if (i_mountPoint == nullptr) {
                m_mountPoints.unlock();
                if (inode != nullptr)
                    *inode = nullptr;
                if (status != nullptr)
                    *status = -ENOSYS;
                return nullptr;
            }
            if (i_mountPoint->RootInode == last_inode) {
                mountPoint = i_mountPoint;
                break;
            }
        }
        m_mountPoints.unlock();
        if (mountPoint == last_mountPoint) {
            if (inode != nullptr)
                *inode = nullptr;
            if (status != nullptr)
                *status = -EINVAL;
            return nullptr;
        }
    }
    if (inode != nullptr)
        *inode = nullptr;
    if (status != nullptr)
        *status = -ENOSYS;
    return nullptr;
}

VFS_MountPoint* VFS::GetChildMountPoint(const char* path, VFS_WorkingDirectory* working_directory, Inode** inode, int* status) const {
    Inode* i_inode = nullptr;
    int rc = 0;
    VFS_MountPoint* parent_mountPoint = GetMountPoint(path, working_directory, &i_inode, &rc);
    if (parent_mountPoint == nullptr || (i_inode == nullptr && rc != ESUCCESS)) {
        if (inode != nullptr)
            *inode = nullptr;
        if (status != nullptr)
            *status = -EINVAL;
        return nullptr;
    }

    VFS_MountPoint* mountPoint = nullptr;
    m_mountPoints.lock();
    for (uint64_t i = 0; i < m_mountPoints.getCount(); i++) {
        VFS_MountPoint* i_mountPoint = m_mountPoints.get(i);
        if (i_mountPoint == nullptr) {
            m_mountPoints.unlock();
            if (inode != nullptr)
                *inode = nullptr;
            if (status != nullptr)
                *status = -ENOSYS;
            return nullptr;
        }
        if (i_mountPoint->RootInode == i_inode) {
            mountPoint = i_mountPoint;
            break;
        }
    }
    m_mountPoints.unlock();
    if (status != nullptr)
        *status = ESUCCESS;
    if (mountPoint == nullptr) {
        if (inode != nullptr)
            *inode = i_inode;
        return parent_mountPoint;
    }
    else {
        if (inode != nullptr)
            *inode = nullptr;
        return mountPoint;
    }
}

bool VFS::isMountpoint(const char* path, size_t len, VFS_WorkingDirectory* working_directory, int* status) {
    if (path == nullptr || len == 0) {
        if (status != nullptr)
            *status = -EINVAL;
        return false;
    }
    Inode* inode = nullptr;
    VFS_MountPoint* mountPoint = GetMountPoint(path, working_directory, &inode);
    if (mountPoint == nullptr) {
        if (status != nullptr)
            *status = -EINVAL;
        return false;
    }
    if (status != nullptr)
        *status = ESUCCESS;
    return inode == mountPoint->RootInode;
}

int VFS::DestroyFileSystem() {
    return -ENOSYS;
}

Inode* VFS::GetRootInode(uint64_t index, int* status) const {
    // just make the root mountpoint deal with it
    if (m_root == nullptr) {
        if (status != nullptr)
            *status = -ENOSYS;
        return nullptr;
    }
    switch (m_root->type) {
        case FileSystemType::TMPFS:
            if (m_root->fs == nullptr) {
                if (status != nullptr)
                    *status = -ENOSYS;
                return nullptr;
            }
            {
                using namespace TempFS;
                TempFileSystem* fs = (TempFileSystem*)m_root->fs;
                return fs->GetRootInode(index, status);
            }
            break;
        default:
            if (status != nullptr)
                *status = -ENOSYS;
            return nullptr;
            break;
    }
}

uint64_t VFS::GetRootInodeCount() const {
    // Make the root mountpoint deal with it
    if (m_root == nullptr) {
        return 0;
    }
    switch (m_root->type) {
        case FileSystemType::TMPFS:
            if (m_root->fs == nullptr) {
                return 0;
            }
            {
                using namespace TempFS;
                TempFileSystem* fs = (TempFileSystem*)m_root->fs;
                return fs->GetRootInodeCount();
            }
            break;
        default:
            return 0;
            break;
    }
}

FileSystemType VFS::GetType() const {
    return FileSystemType::VFS;
}

FilePrivilegeLevel VFS::GetRootPrivilege() const {
    return {0, 0, 00755};
}

