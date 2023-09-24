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

namespace Scheduling {

    Thread::Thread(Process* parent, ThreadEntry_t entry, void* entry_data, uint8_t flags, uint64_t kernel_stack) : m_Parent(parent), m_entry(entry), m_entry_data(entry_data), m_flags(flags), m_stack(0), m_cleanup({nullptr, nullptr}), m_FDManager() {
        fast_memset(&m_regs, 0, DIV_ROUNDUP(sizeof(m_regs), 8));
        m_frame.kernel_stack = kernel_stack;
    }

    Thread::~Thread() {

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

    void Thread::SetKernelStack(uint64_t kernel_stack) {
        m_frame.kernel_stack = kernel_stack;
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

    fd_t Thread::sys$open(const char* path, unsigned long mode) {
        if (m_Parent == nullptr || !m_Parent->ValidateStringRead(path))
            return -EFAULT;

        bool create = mode & O_CREATE;
        if (create)
            mode &= ~O_CREATE;

        FileDescriptorMode new_mode;
        uint8_t vfs_modes;
        if (mode == O_READ) {
            new_mode = FileDescriptorMode::READ;
            vfs_modes = VFS_READ;
        }
        else if (mode == O_APPEND) {
            new_mode = FileDescriptorMode::APPEND;
            vfs_modes = VFS_WRITE;
        }
        else if (mode == O_WRITE) {
            new_mode = FileDescriptorMode::WRITE;
            vfs_modes = VFS_WRITE;
        }
        else if (mode == (O_READ | O_WRITE)) {
            new_mode = FileDescriptorMode::READ_WRITE;
            vfs_modes = VFS_READ | VFS_WRITE;
        }
        else
            return -EINVAL;

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
                bool successful = g_VFS->CreateFile(parent, end == nullptr ? path : child_start);
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
        
        FileStream* stream = g_VFS->OpenStream(path, vfs_modes);
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
        
        if (!descriptor->Read((uint8_t*)buf, count)) {
            switch (descriptor->GetLastError()) {
            case FileDescriptorError::INVALID_MODE:
                return -EBADF;
            default:
                assert(false); // something has gone seriously wrong. just crash. FIXME: handle this case better
            }
        }

        return count; // partial reads are unsupported, so just return the full amount
    }

    long Thread::sys$write(fd_t file, const void* buf, unsigned long count) {
        if (m_Parent == nullptr || !m_Parent->ValidateRead(buf, count))
            return -EFAULT;

        FileDescriptor* descriptor = m_FDManager.GetFileDescriptor(file);
        if (descriptor == nullptr)
            return -EBADF;
        
        if (!descriptor->Write((uint8_t*)buf, count)) {
            switch (descriptor->GetLastError()) {
            case FileDescriptorError::INVALID_MODE:
                return -EBADF;
            case FileDescriptorError::STREAM_ERROR:
                return -ENOMEM;
            default:
                assert(false); // something has gone seriously wrong. just crash. FIXME: handle this case better
            }
        }

        if (descriptor->GetType() == FileDescriptorType::TTY)
            ((TTY*)descriptor->GetData())->GetVGADevice()->SwapBuffers(false);

        return count; // partial writes are unsupported, so just return the full amount
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

    long Thread::sys$seek(fd_t file, long offset) {
        FileDescriptor* descriptor = m_FDManager.GetFileDescriptor(file);
        if (descriptor == nullptr)
            return -EBADF;

        if (offset < 0)
            return -EINVAL;

        if (!descriptor->Seek((uint64_t)offset)) {
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

        return offset;
    }
    
}