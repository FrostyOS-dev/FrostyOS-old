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

#ifndef _FILE_DESCRIPTOR_HPP
#define _FILE_DESCRIPTOR_HPP

#include "FileStream.hpp"

#include <tty/TTY.hpp>

#include <stddef.h>
#include <stdint.h>

#ifndef O_READ
#define O_READ 1UL
#endif
#ifndef O_WRITE
#define O_WRITE 2UL
#endif
#ifndef O_CREATE
#define O_CREATE 4UL
#endif
#ifndef O_APPEND
#define O_APPEND 8UL
#endif

#ifndef fd_t
typedef long fd_t;
#endif


enum class FileDescriptorType {
    UNKNOWN,
    FILE_STREAM,
    TTY,
    DEBUG
};

enum class FileDescriptorMode {
    READ,
    WRITE,
    APPEND,
    READ_WRITE
};

enum class FileDescriptorError {
    SUCCESS = 0,
    INVALID_ARGUMENTS = 1,
    INTERNAL_ERROR = 2,
    STREAM_ERROR = 3,
    INVALID_MODE = 4,
    NO_PERMISSION = 5
};

class FileDescriptor {
public:
    FileDescriptor();
    FileDescriptor(FileDescriptorType type, void* data, FileDescriptorMode mode, fd_t ID);
    ~FileDescriptor();

    bool Open();
    bool Close();

    size_t Read(uint8_t* buffer, size_t count);
    size_t Write(const uint8_t* buffer, size_t count);

    bool Seek(uint64_t offset);
    bool Rewind();

    fd_t GetID() const;

    void* GetData() const;

    FileDescriptorType GetType() const;

    FileDescriptorError GetLastError() const;

protected:
    void SetLastError(FileDescriptorError error) const;

protected:
    mutable FileDescriptorError p_lastError;

private:
    TTY* m_TTY;
    FileStream* m_FileStream;
    bool m_is_open;
    FileDescriptorType m_type;
    FileDescriptorMode m_mode;
    fd_t m_ID;
};

#endif /* _FILE_DESCRIPTOR_HPP */