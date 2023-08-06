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

#ifndef _FILE_STREAM_HPP
#define _FILE_STREAM_HPP

#include "VFS.hpp"
#include "Inode.hpp"

enum class FileStreamError {
    SUCCESS = 0,
    INVALID_ARGUMENTS = 1, // Invalid argument(s). e.g. invalid pointer
    INTERNAL_ERROR = 2, // error in internal data structures
    ALLOCATION_FAILED = 3, // either new, malloc or AllocatePage(s) failed
    INVALID_MODE = 4, // Invalid stream mode. e.g. write attempted on read-only stream
    INVALID_INODE = 5, // Invalid Inode. e.g. a folder Inode or a NULL inode
    INVALID_FS_TYPE = 6, // Invalid FileSystem type
    INVALID_MOUNTPOINT = 7, // Invalid VFS Mount-point
    STREAM_CLOSED = 8 // File stream is closed
};

class FileStream {
public:
    FileStream(Inode* inode, VFS_MountPoint* mountPoint, uint8_t modes);
    ~FileStream();

    bool Open();
    bool Close();
    bool ReadStream(uint8_t* bytes, uint64_t count = 1);
    bool WriteStream(const uint8_t* bytes, uint64_t count = 1);
    bool Seek(uint64_t offset);
    bool Rewind();
    uint64_t GetOffset() const;
    bool isOpen() const;

    FileStreamError GetLastError() const;

private:

    void SetLastError(FileStreamError) const;

private:
    Inode* m_inode;
    VFS_MountPoint* m_mountPoint;
    uint8_t m_modes;

    mutable FileStreamError m_lastError;
};

#endif /* _FILE_STREAM_HPP */