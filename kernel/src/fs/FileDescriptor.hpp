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

#ifndef _FILE_DESCRIPTOR_HPP
#define _FILE_DESCRIPTOR_HPP

#include <tty/TTY.hpp>

#include <stdint.h>
#include <spinlock.h>

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
    DIRECTORY_STREAM,
    TTY
};

enum class FileDescriptorMode {
    READ,
    WRITE,
    APPEND,
    READ_WRITE
};

struct TTYFileDescriptor {
    TTY* tty;
    TTYBackendMode mode;
};

class FileDescriptor {
public:
    FileDescriptor();
    FileDescriptor(FileDescriptorType type, void* data, FileDescriptorMode mode, fd_t ID);
    ~FileDescriptor();

    int Open();
    int Close();

    int64_t Read(uint8_t* buffer, int64_t count, int* status = nullptr); // status will be set if not nullptr, and if the return value is >= 0.
    int64_t Write(const uint8_t* buffer, int64_t count, int* status = nullptr); // status will be set if not nullptr, and if the return value is >= 0.

    int Seek(int64_t offset);
    int Rewind();

    fd_t GetID() const;

    void* GetData() const;

    FileDescriptorType GetType() const;

    bool WasInitSuccessful() const;

    void ForceUnlock(); // should only ever be used in a PANIC to get emergency access to resources.

private:
    TTYFileDescriptor* m_TTYInfo;
    void* m_Stream; // works for both FileStream and DirectoryStream
    bool m_is_open;
    FileDescriptorType m_type;
    FileDescriptorMode m_mode;
    fd_t m_ID;

    bool m_init_successful;

    mutable spinlock_t m_lock;
};

#endif /* _FILE_DESCRIPTOR_HPP */