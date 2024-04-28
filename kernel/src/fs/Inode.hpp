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

#include "FilePrivilegeLevel.hpp"

enum class InodeType {
    Unkown = -1, // normally used for errors
    File = 0,
    Folder = 1,
    SymLink = 2
};


class Inode {
public:
    struct ErrorAndResult {
        int error;
        uint64_t result;
    };

    virtual ~Inode() {}

    virtual int Open() = 0;
    virtual int Close() = 0;
    virtual int64_t ReadStream(FilePrivilegeLevel privilege, uint8_t* bytes, int64_t count = 1) = 0;
    virtual int64_t WriteStream(FilePrivilegeLevel privilege, const uint8_t* bytes, int64_t count = 1) = 0;
    virtual int Seek(int64_t offset) = 0;
    virtual int Rewind() = 0;
    virtual int64_t GetOffset() const { return p_CurrentOffset; }
    virtual bool isOpen() const { return p_isOpen; }

    virtual ErrorAndResult Read(FilePrivilegeLevel privilege, int64_t offset, uint8_t* bytes, int64_t count = 1) = 0;
    virtual ErrorAndResult Write(FilePrivilegeLevel privilege, int64_t offset, const uint8_t* bytes, int64_t count = 1) = 0;

    virtual int Expand(size_t new_size) = 0;

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

    virtual Inode* GetParent() const = 0;

    virtual FilePrivilegeLevel GetPrivilegeLevel() const = 0;
    virtual void SetPrivilegeLevel(FilePrivilegeLevel privilege) = 0;

    virtual uint64_t GetChildCount() const = 0;
    virtual Inode* GetChild(uint64_t index) const = 0;
    virtual Inode* GetChild(const char* name) const = 0;

    virtual void Lock() const = 0;
    virtual void Unlock() const = 0;

protected:
    // Standard members
    char const* p_name;
    InodeType p_type;
    size_t p_blockSize;
    uint64_t p_ID;

    // Stream members
    int64_t p_CurrentOffset;
    bool p_isOpen;
};

#endif /* _INODE_HPP */