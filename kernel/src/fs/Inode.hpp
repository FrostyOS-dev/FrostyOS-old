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

#ifndef _INODE_HPP
#define _INODE_HPP

#include <stdint.h>
#include <stddef.h>

enum class InodeType {
    Unkown = -1, // normally used for errors
    File = 0,
    Folder = 1,
    SymLink = 2
};

enum class InodeError {
    SUCCESS = 0,
    STREAM_CLOSED = 1, // The stream is closed. e.g. ReadStream attempted, but the stream is closed.
    INVALID_ARGUMENTS = 2, // One or more arguments are invalid. e.g. invalid byte count
    INVALID_TYPE = 3, // Invalid operation for Inode. e.g. Read/Write on folder, Adding a child to a file, Unknown Inode type
    ALLOCATION_FAILED = 4, // Memory allocation failed
    INTERNAL_ERROR = 5 // An error with class data structure(s)
};

class Inode {
public:
    virtual ~Inode() {}

    virtual bool Open() = 0;
    virtual bool Close() = 0;
    virtual bool ReadStream(uint8_t* bytes, uint64_t count = 1) = 0;
    virtual bool WriteStream(const uint8_t* bytes, uint64_t count = 1) = 0;
    virtual bool Seek(uint64_t offset) = 0;
    virtual bool Rewind() = 0;
    virtual uint64_t GetOffset() const { return p_CurrentOffset; }
    virtual bool isOpen() const { return p_isOpen; }

    virtual bool Read(uint64_t offset, uint8_t* bytes, uint64_t count = 1) = 0;
    virtual bool Write(uint64_t offset, const uint8_t* bytes, uint64_t count = 1) = 0;

    virtual bool Expand(size_t new_size) = 0;

    virtual const char* GetName() const { return p_name; }
    virtual void SetName(const char* name) { p_name = name; }

    virtual InodeType GetType() const = 0; // get the type of the Inode. for links, this returns the type of the item pointed to by the Inode
    virtual void SetType(InodeType type) = 0;

    virtual InodeType GetRealType() const { return p_type; } // get the real type of the Inode. for links, this actually returns the type
    virtual void SetRealType(InodeType type) { p_type = type; }

    virtual size_t GetBlockSize() const { return p_blockSize; }
    virtual void SetBlockSize(size_t blockSize) { p_blockSize = blockSize; }

    virtual uint64_t GetID() const { return p_ID; }
    virtual void ResetID(uint32_t seed = 0) = 0;

    virtual InodeError GetLastError() const { return p_lastError; }

    virtual Inode* GetParent() const = 0;

protected:
    virtual void SetLastError(InodeError error) const { p_lastError = error; } // const so const functions can perform error reporting

protected:
    // Standard members
    char const* p_name;
    InodeType p_type;
    size_t p_blockSize;
    uint64_t p_ID;

    // Error management
    mutable InodeError p_lastError; // mutable so const functions can perform error reporting

    // Stream members
    uint64_t p_CurrentOffset;
    bool p_isOpen;

};

#endif /* _INODE_HPP */