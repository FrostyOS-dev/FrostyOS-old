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

#include "FileDescriptor.hpp"

#include <math.h>

#include <file.h>

#ifdef __x86_64__
#include <arch/x86_64/E9.h>
#endif

FileDescriptor::FileDescriptor() : m_TTY(nullptr), m_FileStream(nullptr), m_is_open(false), m_type(FileDescriptorType::UNKNOWN), m_mode(FileDescriptorMode::READ), m_ID(-1) {

}

FileDescriptor::FileDescriptor(FileDescriptorType type, void* data, FileDescriptorMode mode, fd_t ID) : m_TTY(nullptr), m_FileStream(nullptr), m_is_open(false), m_type(type), m_mode(mode), m_ID(ID) {
    switch (m_type) {
    case FileDescriptorType::FILE_STREAM:
        m_FileStream = (FileStream*)data;
        break;
    case FileDescriptorType::TTY:
        switch (m_mode) {
        case FileDescriptorMode::READ_WRITE:
        case FileDescriptorMode::WRITE:
            m_TTY = (TTY*)data;
            m_TTY->GetVGADevice()->SetCursorPosition({0,0}); // start at offset 0, but don't clear the screen
            break;
        case FileDescriptorMode::READ:
        case FileDescriptorMode::APPEND:
            m_TTY = (TTY*)data;
            break;
        default:
            SetLastError(FileDescriptorError::INVALID_ARGUMENTS);
            return;
        }
        m_is_open = true;
        break;
    case FileDescriptorType::DEBUG:
        switch (m_mode) {
        case FileDescriptorMode::READ:
        case FileDescriptorMode::READ_WRITE:
            SetLastError(FileDescriptorError::INVALID_ARGUMENTS); // read is not supported by debug
            return;
        case FileDescriptorMode::WRITE:
        case FileDescriptorMode::APPEND:
            break;
        default:
            SetLastError(FileDescriptorError::INVALID_ARGUMENTS);
            return;
        }
        m_is_open = true;
        break;
    default:
        SetLastError(FileDescriptorError::INVALID_ARGUMENTS);
        return;
    }
    SetLastError(FileDescriptorError::SUCCESS);
}

FileDescriptor::~FileDescriptor() {
    if (m_is_open)
        (void)Close(); // return value is irrelevant
}

bool FileDescriptor::Open() {
    switch (m_type) {
    case FileDescriptorType::DEBUG:
    case FileDescriptorType::TTY:
        SetLastError(FileDescriptorError::SUCCESS); // already open
        return true;
    case FileDescriptorType::FILE_STREAM:
        if (!m_FileStream->Open()) {
            SetLastError(FileDescriptorError::INTERNAL_ERROR); // if open fails, something is most likely wrong with the VFS
            return false;
        }
        if (m_mode == FileDescriptorMode::APPEND) {
            if (!m_FileStream->Seek(m_FileStream->GetSize() - 1)) { // seek to end
                SetLastError(FileDescriptorError::INTERNAL_ERROR); // if seeking to a known value fails, something is most likely wrong with the VFS
                return false;
            }
        }
        SetLastError(FileDescriptorError::SUCCESS);
        return true;
    default:
        SetLastError(FileDescriptorError::INTERNAL_ERROR); // this case should have already been caught by the constructor
        return false;
    }
}

bool FileDescriptor::Close() {
    switch (m_type) {
    case FileDescriptorType::DEBUG:
    case FileDescriptorType::TTY:
        SetLastError(FileDescriptorError::SUCCESS); // cannot be closed
        return true;
    case FileDescriptorType::FILE_STREAM:
        if (!m_FileStream->Close()) {
            SetLastError(FileDescriptorError::INTERNAL_ERROR); // if close fails, something is most likely wrong with the VFS
            return false;
        }
        SetLastError(FileDescriptorError::SUCCESS);
        return true;
    default:
        SetLastError(FileDescriptorError::INTERNAL_ERROR); // this case should have already been caught by the constructor
        return false;
    }
}

#include <HAL/hal.hpp>

size_t FileDescriptor::Read(uint8_t* buffer, size_t count) {
    switch (m_mode) {
    case FileDescriptorMode::READ:
    case FileDescriptorMode::READ_WRITE:
        break;
    default:
        SetLastError(FileDescriptorError::INVALID_MODE);
        return 0;
    }
    switch (m_type) {
    case FileDescriptorType::TTY: {
        uint64_t i = 0;
        while (i < count) {
            int c = m_TTY->getc();
            if (c == EOF)
                break;
            buffer[i] = (uint8_t)c;
            i++;
        }
        if (i < count)
            SetLastError(FileDescriptorError::STREAM_ERROR);
        else
            SetLastError(FileDescriptorError::SUCCESS);
        return i; // NOTE: partial reads from TTYs mean the keyboard device had an error or was disconnected.
    }
    case FileDescriptorType::DEBUG:
        SetLastError(FileDescriptorError::INTERNAL_ERROR); // this case should have already been caught by the constructor
        return 0;
    case FileDescriptorType::FILE_STREAM: {
        size_t status = m_FileStream->ReadStream(buffer, count);
        if (status != count) {
            switch (m_FileStream->GetLastError()) {
            case FileStreamError::INVALID_ARGUMENTS:
                SetLastError(FileDescriptorError::INVALID_ARGUMENTS);
                break;
            case FileStreamError::STREAM_CLOSED:
                SetLastError(FileDescriptorError::STREAM_ERROR);
                break;
            default:
                SetLastError(FileDescriptorError::INTERNAL_ERROR);
                break;
            }
        }
        else
            SetLastError(FileDescriptorError::SUCCESS);
        return status;
    }
    default:
        SetLastError(FileDescriptorError::INTERNAL_ERROR); // this case should have already been caught by the constructor
        return 0;
    }
}

size_t FileDescriptor::Write(const uint8_t* buffer, size_t count) {
    switch (m_mode) {
    case FileDescriptorMode::APPEND:
    case FileDescriptorMode::WRITE:
    case FileDescriptorMode::READ_WRITE:
        break;
    default:
        SetLastError(FileDescriptorError::INVALID_MODE);
        return 0;
    }
    switch (m_type) {
    case FileDescriptorType::TTY: {
        uint64_t i = 0;
        while (i < count) {
            m_TTY->putc(buffer[i]);
            i++;
        }
        //m_TTY->GetVGADevice()->SwapBuffers();
        SetLastError(FileDescriptorError::SUCCESS);
        return count; // partial writes to TTYs are not supported
    }
    case FileDescriptorType::FILE_STREAM: {
        uint64_t status = m_FileStream->WriteStream(buffer, count);
        if (status != count) {
            switch (m_FileStream->GetLastError()) {
            case FileStreamError::INVALID_ARGUMENTS:
                SetLastError(FileDescriptorError::INVALID_ARGUMENTS);
                break;
            case FileStreamError::ALLOCATION_FAILED:
            case FileStreamError::STREAM_CLOSED:
                SetLastError(FileDescriptorError::STREAM_ERROR);
                break;
            default:
                SetLastError(FileDescriptorError::INTERNAL_ERROR);
                break;
            }
        }
        else
            SetLastError(FileDescriptorError::SUCCESS);
        return status;
    }
    case FileDescriptorType::DEBUG: {
        uint64_t i = 0;
#ifndef NDEBUG
        while (i < count) {
#ifdef __x86_64__
            x86_64_debug_putc(buffer[i]);
#endif
            i++;
        }
#endif
        SetLastError(FileDescriptorError::SUCCESS);
        return i;
    }
    default:
        SetLastError(FileDescriptorError::INTERNAL_ERROR); // this case should have already been caught by the constructor
        return 0;
    }
}

bool FileDescriptor::Seek(uint64_t offset) {
    switch (m_mode) {
    case FileDescriptorMode::READ:
    case FileDescriptorMode::WRITE:
    case FileDescriptorMode::READ_WRITE:
        break;
    default:
        SetLastError(FileDescriptorError::INVALID_MODE);
        return false;
    }
    switch (m_type) {
    case FileDescriptorType::DEBUG:
        SetLastError(FileDescriptorError::STREAM_ERROR); // unsupported
        return false;
    case FileDescriptorType::TTY: {
        uldiv_t out = uldiv(offset, m_TTY->GetVGADevice()->GetAmountOfTextRows());
        m_TTY->GetVGADevice()->SetCursorPosition({out.rem * 10, out.quot * 16});
        SetLastError(FileDescriptorError::SUCCESS);
        return true;
    }
    case FileDescriptorType::FILE_STREAM:
        if (!m_FileStream->Seek(offset)) {
            switch (m_FileStream->GetLastError()) {
            case FileStreamError::INVALID_ARGUMENTS:
                SetLastError(FileDescriptorError::INVALID_ARGUMENTS);
                break;
            case FileStreamError::STREAM_CLOSED:
                SetLastError(FileDescriptorError::STREAM_ERROR);
                break;
            default:
                SetLastError(FileDescriptorError::INTERNAL_ERROR);
                break;
            }
            return false;
        }
        SetLastError(FileDescriptorError::SUCCESS);
        return true;
    default:
        SetLastError(FileDescriptorError::INTERNAL_ERROR); // this case should have already been caught by the constructor
        return false;
    }
}

bool FileDescriptor::Rewind() {
    switch (m_mode) {
    case FileDescriptorMode::READ:
    case FileDescriptorMode::WRITE:
    case FileDescriptorMode::READ_WRITE:
        break;
    default:
        SetLastError(FileDescriptorError::INVALID_MODE);
        return false;
    }
    switch (m_type) {
    case FileDescriptorType::DEBUG:
        SetLastError(FileDescriptorError::STREAM_ERROR); // unsupported
        return false;
    case FileDescriptorType::TTY:
        m_TTY->putc('\f'); // clear the screen
        SetLastError(FileDescriptorError::SUCCESS); // this case should have already been caught by the constructor
        return true;
    case FileDescriptorType::FILE_STREAM:
        if (!m_FileStream->Rewind()) {
            switch (m_FileStream->GetLastError()) {
            case FileStreamError::STREAM_CLOSED:
                SetLastError(FileDescriptorError::STREAM_ERROR);
                break;
            default:
                SetLastError(FileDescriptorError::INTERNAL_ERROR);
                break;
            }
            return false;
        }
        SetLastError(FileDescriptorError::SUCCESS);
        return true;
    default:
        SetLastError(FileDescriptorError::INTERNAL_ERROR); // this case should have already been caught by the constructor
        return false;
    }
}

fd_t FileDescriptor::GetID() const {
    SetLastError(FileDescriptorError::SUCCESS);
    return m_ID;
}

void* FileDescriptor::GetData() const {
    SetLastError(FileDescriptorError::SUCCESS);
    switch (m_type) {
    case FileDescriptorType::FILE_STREAM:
        return m_FileStream;
    case FileDescriptorType::TTY:
        return m_TTY;
    case FileDescriptorType::DEBUG:
        return nullptr; // has no data
    default:
        SetLastError(FileDescriptorError::INTERNAL_ERROR);
        return nullptr;
    }
}

FileDescriptorType FileDescriptor::GetType() const {
    SetLastError(FileDescriptorError::SUCCESS);
    return m_type;
}

FileDescriptorError FileDescriptor::GetLastError() const {
    return p_lastError;
}

void FileDescriptor::SetLastError(FileDescriptorError error) const {
    p_lastError = error;
}
