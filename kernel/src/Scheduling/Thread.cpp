/*
Copyright (©) 2022-2023  Frosty515

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

#include "Thread.hpp"

#include "Scheduler.hpp"

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <util.h>

#include <file.h>

#include <fs/TempFS/TempFSInode.hpp>

namespace Scheduling {

    Thread::Thread(Process* parent, ThreadEntry_t entry, void* entry_data, uint8_t flags, tid_t TID) : m_Parent(parent), m_entry(entry), m_entry_data(entry_data), m_flags(flags), m_stack(0), m_cleanup({nullptr, nullptr}), m_FDManager(), m_TID(TID) {
        fast_memset(&m_regs, 0, DIV_ROUNDUP(sizeof(m_regs), 8));
        m_frame.kernel_stack = (uint64_t)g_KPM->AllocatePages(KERNEL_STACK_SIZE >> 12, PagePermissions::READ_WRITE) + KERNEL_STACK_SIZE; // FIXME: use actual page size
    }

    Thread::~Thread() {
        g_KPM->FreePages((void*)(m_frame.kernel_stack - KERNEL_STACK_SIZE));
    }

    void Thread::SetEntry(ThreadEntry_t entry, void* entry_data) {
        m_entry = entry;
        m_entry_data = entry_data;
    }

    void Thread::SetFlags(uint8_t flags) {
        m_flags = flags;
    }

    void Thread::SetParent(Process* parent) {
        m_Parent = parent;
    }

    void Thread::SetStack(uint64_t stack) {
        m_stack = stack;
        if (m_Parent != nullptr) {
            if (m_Parent->GetPriority() == Priority::KERNEL)
                m_frame.kernel_stack = stack;
        }
    }

    void Thread::SetCleanupFunction(ThreadCleanup_t cleanup) {
        m_cleanup = cleanup;
    }

    ThreadEntry_t Thread::GetEntry() const {
        return m_entry;
    }

    void* Thread::GetEntryData() const {
        return m_entry_data;
    }

    uint8_t Thread::GetFlags() const {
        return m_flags;
    }

    Process* Thread::GetParent() const {
        return m_Parent;
    }

    CPU_Registers* Thread::GetCPURegisters() const {
        return &m_regs;
    }

    uint64_t Thread::GetStack() const {
        return m_stack;
    }

    uint64_t Thread::GetKernelStack() const {
        return m_frame.kernel_stack;
    }

    ThreadCleanup_t Thread::GetCleanupFunction() const {
        return m_cleanup;
    } 

    void* Thread::GetStackRegisterFrame() const {
        return &m_frame;
    }

    void Thread::Start() {
        m_FDManager.ReserveFileDescriptor(FileDescriptorType::TTY, g_CurrentTTY, FileDescriptorMode::READ, 0); // not properly supported yet, but here to reserve the file descriptor ID
        Position pos = g_CurrentTTY->GetVGADevice()->GetCursorPosition(); // save the current position
        m_FDManager.ReserveFileDescriptor(FileDescriptorType::TTY, g_CurrentTTY, FileDescriptorMode::WRITE, 1);
        m_FDManager.ReserveFileDescriptor(FileDescriptorType::TTY, g_CurrentTTY, FileDescriptorMode::WRITE, 2);
        g_CurrentTTY->GetVGADevice()->SetCursorPosition(pos); // restore the position
        m_FDManager.ReserveFileDescriptor(FileDescriptorType::DEBUG, nullptr, FileDescriptorMode::APPEND, 3);
        Scheduler::ScheduleThread(this);
    }

    fd_t Thread::sys$open(const char* path, unsigned long flags, unsigned short mode) {
        if (m_Parent == nullptr || !m_Parent->ValidateStringRead(path))
            return -EFAULT;

        bool create = flags & O_CREATE;
        if (create)
            flags &= ~O_CREATE;

        FileDescriptorMode new_mode;
        uint8_t vfs_modes;
        if (flags == O_READ) {
            new_mode = FileDescriptorMode::READ;
            vfs_modes = VFS_READ;
        }
        else if (flags == O_APPEND) {
            new_mode = FileDescriptorMode::APPEND;
            vfs_modes = VFS_WRITE;
        }
        else if (flags == O_WRITE) {
            new_mode = FileDescriptorMode::WRITE;
            vfs_modes = VFS_WRITE;
        }
        else if (flags == (O_READ | O_WRITE)) {
            new_mode = FileDescriptorMode::READ_WRITE;
            vfs_modes = VFS_READ | VFS_WRITE;
        }
        else
            return -EINVAL;

        FilePrivilegeLevel current_privilege = {m_Parent->GetEUID(), m_Parent->GetEGID(), 0};

        bool valid_path = g_VFS->IsValidPath(path);
        if (create) {
            if (!valid_path) {
                char* end = strrchr(path, PATH_SEPARATOR);
                char* child_start = (char*)((uint64_t)end + sizeof(char));
                char const* parent;
                if (end != nullptr) {
                    size_t parent_name_size = (size_t)child_start - (size_t)path;
                    char* i_parent = new char[parent_name_size + 1];
                    memcpy(i_parent, path, parent_name_size);
                    i_parent[parent_name_size] = '\0';
                    if (!g_VFS->IsValidPath(path)) {
                        delete[] i_parent;
                        return -ENOENT;
                    }
                    parent = i_parent;
                }
                else
                    parent = "/";
                bool successful = g_VFS->CreateFile(current_privilege, parent, end == nullptr ? path : child_start, 0, false, {m_Parent->GetEUID(), m_Parent->GetEGID(), mode});
                if (end != nullptr)
                    delete[] parent;
                if (!successful) {
                    switch (g_VFS->GetLastError()) {
                    case FileSystemError::INVALID_ARGUMENTS:
                        return -ENOTDIR;
                    case FileSystemError::ALLOCATION_FAILED:
                        return -ENOMEM;
                    default:
                        assert(false); // something has gone seriously wrong. just crash. FIXME: handle this case better
                    }
                }
            }
        }
        else if (!valid_path)
            return -ENOENT;
        
        FileStream* stream = g_VFS->OpenStream(current_privilege, path, vfs_modes);
        if (stream == nullptr) {
            switch (g_VFS->GetLastError()) {
            case FileSystemError::ALLOCATION_FAILED:
                return -ENOMEM;
            default:
                assert(false); // something has gone seriously wrong. just crash. FIXME: handle this case better
            }
        }

        fd_t fd = m_FDManager.AllocateFileDescriptor(FileDescriptorType::FILE_STREAM, stream, new_mode);
        if (fd < 0) {
            (void)(g_VFS->CloseStream(stream)); // we are cleaning up, return value is irrelevant
            return -ENOMEM;
        }

        FileDescriptor* descriptor = m_FDManager.GetFileDescriptor(fd);
        if (descriptor == nullptr) {
            (void)(m_FDManager.FreeFileDescriptor(fd));
            (void)(g_VFS->CloseStream(stream)); // we are cleaning up, return value is irrelevant
            assert(false); // something has gone seriously wrong. just crash. FIXME: handle this case better
        }

        if (!descriptor->Open()) {
            assert(false); // something has gone seriously wrong. just crash. FIXME: handle this case better
        }

        return fd;
    }

    long Thread::sys$read(fd_t file, void* buf, unsigned long count) {
        if (m_Parent == nullptr || !m_Parent->ValidateWrite(buf, count))
            return -EFAULT;

        FileDescriptor* descriptor = m_FDManager.GetFileDescriptor(file);
        if (descriptor == nullptr)
            return -EBADF;
        
        size_t status = descriptor->Read((uint8_t*)buf, count);
        if (status == 0) {
            switch (descriptor->GetLastError()) {
            case FileDescriptorError::INVALID_ARGUMENTS:
                return EOF;
            case FileDescriptorError::INVALID_MODE:
            case FileDescriptorError::STREAM_ERROR:
                return -EBADF;
            case FileDescriptorError::NO_PERMISSION:
                return -EACCES;
            default: {
                PANIC("File descriptor internal error in read syscall.");
            }
            }
        }
        return status;
    }

    long Thread::sys$write(fd_t file, const void* buf, unsigned long count) {
        if (count == 0)
            return ESUCCESS; // no point as there is nothing to write

        if (m_Parent == nullptr || !m_Parent->ValidateRead(buf, count))
            return -EFAULT;

        FileDescriptor* descriptor = m_FDManager.GetFileDescriptor(file);
        if (descriptor == nullptr)
            return -EBADF;
        
        size_t status = descriptor->Write((const uint8_t*)buf, count);
        if (status == 0) {
            switch (descriptor->GetLastError()) {
            case FileDescriptorError::INVALID_ARGUMENTS:
                return EOF;
            case FileDescriptorError::INVALID_MODE:
            case FileDescriptorError::STREAM_ERROR: {
                /* This gets complicated because we have no simple way to know if it is a allocation failure or a bad file descriptor.
                We must check if the descriptor is a FileStream, and if it is, check if an allocation failure occurred.
                If not, we can assume it is a bad file descriptor.
                */
                if (descriptor->GetType() == FileDescriptorType::FILE_STREAM) {
                    FileStream* stream = (FileStream*)descriptor->GetData();
                    if (stream != nullptr) {
                        switch (stream->GetLastError()) {
                        case FileStreamError::ALLOCATION_FAILED:
                            return -ENOMEM;
                        default:
                            return -EBADF;
                        }
                    }
                }
                return -EBADF;
            }
            case FileDescriptorError::NO_PERMISSION:
                return -EACCES;
            default: {
                dbgprintf("File descriptor internal error %d in write syscall. fd = %ld, buf = %lp, count = %lu\n", (int)descriptor->GetLastError(), file, buf, count);
                PANIC("File descriptor internal error in write syscall.");
            }
            }
        }

        if (descriptor->GetType() == FileDescriptorType::TTY)
            ((TTY*)descriptor->GetData())->GetVGADevice()->SwapBuffers(false);

        return status;
    }

    int Thread::sys$close(fd_t file) {
        FileDescriptor* descriptor = m_FDManager.GetFileDescriptor(file);
        if (descriptor == nullptr)
            return -EBADF;

        (void)descriptor->Close(); // return value is irrelevant
        if (descriptor->GetType() == FileDescriptorType::FILE_STREAM) {
            FileStream* stream = (FileStream*)descriptor->GetData();
            if (stream != nullptr)
                delete stream;
        }
        (void)m_FDManager.FreeFileDescriptor(file); // return value is irrelevant
        delete descriptor;
        return ESUCCESS;
    }

    long Thread::sys$seek(fd_t file, long offset, long whence) {
        FileDescriptor* descriptor = m_FDManager.GetFileDescriptor(file);
        if (descriptor == nullptr || descriptor->GetType() != FileDescriptorType::FILE_STREAM)
            return -EBADF;

        FileStream* stream = (FileStream*)descriptor->GetData();
        if (stream == nullptr)
            return -EBADF;

        long i_offset;
        if (whence == SEEK_SET) {
            if (offset < 0)
                return -EINVAL;
            i_offset = offset;
        }
        else if (whence == SEEK_CUR)
                i_offset = stream->GetOffset() + offset;
        else if (whence == SEEK_END) {
            Inode* inode = stream->GetInode();
            FileSystem* fs = stream->GetFileSystem();
            if (inode == nullptr || fs == nullptr)
                return -EBADF;
            switch (fs->GetType()) {
            case FileSystemType::TMPFS: {
                TempFS::TempFSInode* tempfs_inode = (TempFS::TempFSInode*)inode;
                if (tempfs_inode->GetType() == InodeType::File)
                    i_offset = tempfs_inode->GetSize() - 1 + offset;
                else
                    return -EINVAL;
                break;
            }
            default:
                return -ENOSYS;
            }
        }
        else
            return -EINVAL;

        if (!descriptor->Seek((uint64_t)i_offset)) {
            switch (descriptor->GetLastError()) {
            case FileDescriptorError::INVALID_ARGUMENTS:
                return -EINVAL;
            case FileDescriptorError::INVALID_MODE:
            case FileDescriptorError::STREAM_ERROR:
                return -EBADF;
            default:
                assert(false); // something has gone seriously wrong. just crash. FIXME: handle this case better
            }
        }

        return i_offset;
    }

    int Thread::sys$stat(const char* path, struct stat_buf* buf) {
        if (m_Parent == nullptr || !m_Parent->ValidateWrite(buf, sizeof(struct stat_buf)) || !m_Parent->ValidateStringRead(path))
            return -EFAULT;

        // Validate path
        if (!g_VFS->IsValidPath(path))
            return -ENOENT;

        FileSystem* fs = nullptr;
        Inode* inode = g_VFS->GetInode(path, &fs);
        if (inode == nullptr || fs == nullptr)
            return -ENOENT;

        FilePrivilegeLevel privilege = inode->GetPrivilegeLevel();
        
        struct stat_buf buffer;

        buffer.st_uid = privilege.UID;
        buffer.st_gid = privilege.GID;
        buffer.st_mode = privilege.ACL;

        switch (fs->GetType()) {
        case FileSystemType::TMPFS: {
            TempFS::TempFSInode* tempfs_inode = (TempFS::TempFSInode*)inode;
            if (tempfs_inode->GetType() == InodeType::File)
                buffer.st_size = tempfs_inode->GetSize();
            else
                buffer.st_size = 0;
            break;
        }
        default:
            return -ENOSYS;
        }

        memcpy(buf, &buffer, sizeof(struct stat_buf));
        return ESUCCESS;
    }

    int Thread::sys$fstat(fd_t file, struct stat_buf* buf) {
        if (m_Parent == nullptr || !m_Parent->ValidateWrite(buf, sizeof(struct stat_buf)))
            return -EFAULT;

        FileDescriptor* descriptor = m_FDManager.GetFileDescriptor(file);
        if (descriptor == nullptr || descriptor->GetType() != FileDescriptorType::FILE_STREAM)
            return -EBADF;

        FileStream* stream = (FileStream*)descriptor->GetData();
        if (stream == nullptr)
            return -EBADF;

        Inode* inode = stream->GetInode();
        FileSystem* fs = stream->GetFileSystem();
        if (inode == nullptr || fs == nullptr)
            return -EBADF;

        FilePrivilegeLevel privilege = inode->GetPrivilegeLevel();
        
        struct stat_buf buffer;

        buffer.st_uid = privilege.UID;
        buffer.st_gid = privilege.GID;
        buffer.st_mode = privilege.ACL;

        switch (fs->GetType()) {
        case FileSystemType::TMPFS: {
            TempFS::TempFSInode* tempfs_inode = (TempFS::TempFSInode*)inode;
            if (tempfs_inode->GetType() == InodeType::File)
                buffer.st_size = tempfs_inode->GetSize();
            else
                buffer.st_size = 0;
            break;
        }
        default:
            return -ENOSYS;
        }

        memcpy(buf, &buffer, sizeof(struct stat_buf));
        return ESUCCESS;
    }

    int Thread::sys$chown(const char* path, unsigned int uid, unsigned int gid) {
        if (m_Parent == nullptr || !m_Parent->ValidateStringRead(path))
            return -EFAULT;
        
        if (uid == (unsigned int)-1 && gid == (unsigned int)-1)
            return ESUCCESS; // no point checking anything if we are not changing anything

        // Ensure we are UID 0
        if (m_Parent->GetEUID() != 0)
            return -EPERM;

        // Validate path
        if (!g_VFS->IsValidPath(path))
            return -ENOENT;

        Inode* inode = g_VFS->GetInode(path);
        if (inode == nullptr)
            return -ENOENT;

        FilePrivilegeLevel privilege = inode->GetPrivilegeLevel();
        if (uid != (unsigned int)-1)
            privilege.UID = uid;
        if (gid != (unsigned int)-1)
            privilege.GID = gid;
        inode->SetPrivilegeLevel(privilege);
        return ESUCCESS;
    }

    int Thread::sys$fchown(fd_t file, unsigned int uid, unsigned int gid) {
        if (m_Parent == nullptr)
            return -EFAULT;

        if (uid == (unsigned int)-1 && gid == (unsigned int)-1)
            return ESUCCESS; // no point checking anything if we are not changing anything

        // Ensure we are UID 0
        if (m_Parent->GetEUID() != 0)
            return -EPERM;

        FileDescriptor* descriptor = m_FDManager.GetFileDescriptor(file);
        if (descriptor == nullptr || descriptor->GetType() != FileDescriptorType::FILE_STREAM)
            return -EBADF;

        FileStream* stream = (FileStream*)descriptor->GetData();
        if (stream == nullptr)
            return -EBADF;

        Inode* inode = stream->GetInode();
        if (inode == nullptr)
            return -EBADF;

        FilePrivilegeLevel privilege = inode->GetPrivilegeLevel();
        if (uid != (unsigned int)-1)
            privilege.UID = uid;
        if (gid != (unsigned int)-1)
            privilege.GID = gid;
        inode->SetPrivilegeLevel(privilege);
        return ESUCCESS;
    }

    int Thread::sys$chmod(const char* path, unsigned short mode) {
        if (m_Parent == nullptr || !m_Parent->ValidateStringRead(path))
            return -EFAULT;

        // Validate path
        if (!g_VFS->IsValidPath(path))
            return -ENOENT;

        Inode* inode = g_VFS->GetInode(path);
        if (inode == nullptr)
            return -ENOENT;

        FilePrivilegeLevel privilege = inode->GetPrivilegeLevel();
        
        // Ensure we are the owner or group owner of the file or we are UID/GID 0
        uint32_t uid = m_Parent->GetEUID();
        uint32_t gid = m_Parent->GetEGID();
        if ((privilege.UID != uid && uid != 0) || (privilege.GID != gid && gid != 0))
            return -EPERM;

        privilege.ACL = mode;
        inode->SetPrivilegeLevel(privilege);
        return ESUCCESS;
    }

    int Thread::sys$fchmod(fd_t file, unsigned short mode) {
        if (m_Parent == nullptr)
            return -EFAULT;

        FileDescriptor* descriptor = m_FDManager.GetFileDescriptor(file);
        if (descriptor == nullptr || descriptor->GetType() != FileDescriptorType::FILE_STREAM)
            return -EBADF;

        FileStream* stream = (FileStream*)descriptor->GetData();
        if (stream == nullptr)
            return -EBADF;

        Inode* inode = stream->GetInode();
        if (inode == nullptr)
            return -EBADF;

        FilePrivilegeLevel privilege = inode->GetPrivilegeLevel();

        // Ensure we are the owner or group owner of the file or we are UID/GID 0
        uint32_t uid = m_Parent->GetEUID();
        uint32_t gid = m_Parent->GetEGID();
        if ((privilege.UID != uid && uid != 0) || (privilege.GID != gid && gid != 0))
            return -EPERM;
        
        privilege.ACL = mode;
        inode->SetPrivilegeLevel(privilege);
        return ESUCCESS;
    }

    void Thread::sys$sleep(unsigned long s) {
        Scheduler::SleepThread(this, s * 1000);
    }

    void Thread::sys$msleep(unsigned long ms) {
        Scheduler::SleepThread(this, ms);
    }

    void Thread::PrintInfo(fd_t file) const {
        fprintf(file, "Thread %lp\n", this);
        fprintf(file, "Entry: %lp\n", m_entry);
        fprintf(file, "Entry data: %lp\n", m_entry_data);
        fprintf(file, "Flags: %u\n", m_flags);
        fprintf(file, "Parent: %lp\n", m_Parent);
        fprintf(file, "Stack: %lp\n", m_stack);
        fprintf(file, "Kernel stack: %lp\n", m_frame.kernel_stack);
        fprintf(file, "Cleanup function: %lp\n", m_cleanup.function);
        fprintf(file, "Cleanup data: %lp\n", m_cleanup.data);
        fprintf(file, "Registers:\n");
        fprintf(file, "RAX: %016lx  ", m_regs.RAX);
        fprintf(file, "RBX: %016lx  ", m_regs.RBX);
        fprintf(file, "RCX: %016lx  ", m_regs.RCX);
        fprintf(file, "RDX: %016lx\n", m_regs.RDX);
        fprintf(file, "RSP: %016lx  ", m_regs.RSP);
        fprintf(file, "RBP: %016lx  ", m_regs.RBP);
        fprintf(file, "RDI: %016lx  ", m_regs.RDI);
        fprintf(file, "RSI: %016lx\n", m_regs.RSI);
        fprintf(file, "R8 : %016lx  ", m_regs.R8);
        fprintf(file, "R9 : %016lx  ", m_regs.R9);
        fprintf(file, "R10: %016lx  ", m_regs.R10);
        fprintf(file, "R11: %016lx\n", m_regs.R11);
        fprintf(file, "R12: %016lx  ", m_regs.R12);
        fprintf(file, "R13: %016lx  ", m_regs.R13);
        fprintf(file, "R14: %016lx  ", m_regs.R14);
        fprintf(file, "R15: %016lx\n", m_regs.R15);
        fprintf(file, "RIP: %016lx  ", m_regs.RIP);
        fprintf(file, "RFLAGS: %016lx\n", m_regs.RFLAGS);
        fprintf(file, "CS: %04lx  ", m_regs.CS);
        fprintf(file, "DS: %04lx  ", m_regs.DS);
        fprintf(file, "SS: %04lx  ", m_regs.DS);
        fprintf(file, "ES: %04lx  ", m_regs.DS);
        fprintf(file, "FS: %04lx  ", m_regs.DS);
        fprintf(file, "GS: %04lx\n", m_regs.DS);
        fprintf(file, "CR3: %016lx\n", m_regs.CR3);
    }

    void Thread::SetTID(tid_t TID) {
        m_TID = TID;
    }

    tid_t Thread::GetTID() const {
        return m_TID;
    }

    bool Thread::IsSleeping() const {
        return m_sleeping;
    }

    void Thread::SetSleeping(bool sleeping) {
        m_sleeping = sleeping;
    }

    uint64_t Thread::GetRemainingSleepTime() const {
        return m_remaining_sleep_time;
    }

    void Thread::SetRemainingSleepTime(uint64_t remaining_sleep_time) {
        m_remaining_sleep_time = remaining_sleep_time;
    }
}