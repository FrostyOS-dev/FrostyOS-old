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

#include "exec.hpp"

#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#include <ProgramLoader/ELF.hpp>

#include <fs/VFS.hpp>
#include <fs/FileStream.hpp>
#include <fs/FilePrivilegeLevel.hpp>

int sys$exec(Scheduling::Process* parent, const char *path, char *const argv[], char *const envv[]) {
    if (parent == nullptr || !parent->ValidateStringRead(path) || !parent->ValidateRead(argv, sizeof(char*)) || !parent->ValidateRead(envv, sizeof(char*)))
        return -EFAULT;

    int argc;
    for (argc = 0; argv[argc] != nullptr; argc++) {
        if (!parent->ValidateStringRead(argv[argc]))
            return -EFAULT;
        if (!parent->ValidateRead(&(argv[argc + 1]), sizeof(char*)))
            return -EFAULT;
    }

    int envc;
    for (envc = 0; envv[envc] != nullptr; envc++) {
        if (!parent->ValidateStringRead(envv[envc]))
            return -EFAULT;
        if (!parent->ValidateRead(&(envv[envc + 1]), sizeof(char*)))
            return -EFAULT;
    }

    // Copy argv and envv to the kernel
    char** argv_k = new char*[argc + 1];
    char** envv_k = new char*[envc + 1];
    for (int i = 0; i < argc; i++) {
        argv_k[i] = new char[strlen(argv[i]) + 1];
        strcpy(argv_k[i], argv[i]);
    }
    argv_k[argc] = nullptr;
    for (int i = 0; i < envc; i++) {
        envv_k[i] = new char[strlen(envv[i]) + 1];
        strcpy(envv_k[i], envv[i]);
    }
    envv_k[envc] = nullptr;

    int rc = Execute(parent, path, argc, argv_k, envc, envv_k);
    
    for (int i = 0; i < argc; i++)
        delete[] argv_k[i];

    for (int i = 0; i < envc; i++)
        delete[] envv_k[i];

    delete[] argv_k;
    delete[] envv_k;

    return rc;
}

int Execute(Scheduling::Process* parent, const char *path, int argc, char *const argv[], int envc, char *const envv[], Scheduling::Priority priority) {
    if (!g_VFS->IsValidPath(path))
        return -ENOENT;

    uint32_t uid = parent->GetEUID();
    uint32_t gid = parent->GetEGID();

    FileStream* stream = g_VFS->OpenStream({uid, gid, 07777}, path, VFS_READ);
    if (stream == nullptr) {
        switch (g_VFS->GetLastError()) {
        case FileSystemError::ALLOCATION_FAILED:
            return -ENOMEM;
        default:
            assert(false); // something has gone seriously wrong. just crash. FIXME: handle this case better
        }
    }

    Inode* inode = stream->GetInode();
    if (inode == nullptr)
        return -EACCES; // can't check file permissions

    FilePrivilegeLevel privilege = inode->GetPrivilegeLevel();
    if (uid == privilege.UID) {
        if (!((privilege.ACL & ACL_USER_EXECUTE) > 0))
            return -EACCES;
    }
    else if (gid == privilege.GID) {
        if (!((privilege.ACL & ACL_GROUP_EXECUTE) > 0))
            return -EACCES;
    }
    else {
        if (!((privilege.ACL & ACL_OTHER_EXECUTE) > 0))
            return -EACCES;
    }

    assert(stream->Open());

    size_t size = stream->GetSize();
    if (!(size > 0)) {
        stream->Close();
        return -ENOEXEC;
    }

    uint8_t* buffer = new uint8_t[size];

    assert(size == stream->ReadStream(buffer, size)); // This should NEVER fail under these conditions.

    stream->Close();

    g_VFS->CloseStream(stream);


    ELF_Executable* exe = new ELF_Executable(buffer, size);
    ELF_entry_data entry_data = {argc, (char**)argv, envc, (char**)envv};
    if (!exe->Load(&entry_data)) {
        ELFError error = exe->GetLastError();
        delete exe;
        delete[] buffer;
        switch (error) {
        case ELFError::ALLOCATION_FAILED:
            return -ENOMEM;
        case ELFError::INVALID_ELF:
            return -ENOEXEC;
        default:
            assert(false); // something has gone seriously wrong. just crash. FIXME: handle this case better
        }
    }

    VFS_WorkingDirectory* wd;
    VFS_WorkingDirectory* parent_wd = parent->GetDefaultWorkingDirectory();
    if (parent_wd == nullptr)
        wd = new VFS_WorkingDirectory();
    else
        wd = new VFS_WorkingDirectory(*parent_wd);

    pid_t child_pid = exe->Execute(priority, wd);
    assert(exe->GetLastError() == ELFError::SUCCESS);

    return child_pid;
}