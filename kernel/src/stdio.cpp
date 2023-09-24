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

#include "stdio.h"

#include <string.h> // just used for strlen function
#include <errno.h>

#include <tty/TTY.hpp>

#include <fs/FileDescriptorManager.hpp>

void internal_read(fd_t file, void* data, size_t size) {
    FileDescriptor* descriptor = g_KFDManager->GetFileDescriptor(file);
    if (descriptor == nullptr)
        return;
    descriptor->Read((uint8_t*)data, size);
}

void internal_write(fd_t file, const void* data, size_t size) {
    FileDescriptor* descriptor = g_KFDManager->GetFileDescriptor(file);
    if (descriptor == nullptr)
        return;
    descriptor->Write((const uint8_t*)data, size);
}

fd_t internal_open(const char* path, unsigned long mode) {
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

    fd_t fd = g_KFDManager->AllocateFileDescriptor(FileDescriptorType::FILE_STREAM, stream, new_mode);
    if (fd < 0) {
        (void)(g_VFS->CloseStream(stream)); // we are cleaning up, return value is irrelevant
        return -ENOMEM;
    }

    FileDescriptor* descriptor = g_KFDManager->GetFileDescriptor(fd);
    if (descriptor == nullptr) {
        (void)(g_KFDManager->FreeFileDescriptor(fd));
        (void)(g_VFS->CloseStream(stream)); // we are cleaning up, return value is irrelevant
        assert(false); // something has gone seriously wrong. just crash. FIXME: handle this case better
    }

    if (!descriptor->Open()) {
        assert(false); // something has gone seriously wrong. just crash. FIXME: handle this case better
    }

    return fd;
}

int internal_close(fd_t file) {
    FileDescriptor* descriptor = g_KFDManager->GetFileDescriptor(file);
    if (descriptor == nullptr)
        return -EBADF;

    (void)descriptor->Close(); // return value is irrelevant
    if (descriptor->GetType() == FileDescriptorType::FILE_STREAM) {
        FileStream* stream = (FileStream*)descriptor->GetData();
        if (stream != nullptr)
            delete stream;
    }
    (void)g_KFDManager->FreeFileDescriptor(file); // return value is irrelevant
    delete descriptor;
    return ESUCCESS;
}

long internal_seek(fd_t file, long offset) {
    FileDescriptor* descriptor = g_KFDManager->GetFileDescriptor(file);
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

extern "C" void putc(const char c) {
    internal_write(stdout, &c, 1);
    g_CurrentTTY->GetVGADevice()->SwapBuffers();
}

extern "C" void puts(const char* str) {
    internal_write(stdout, str, strlen(str));
    g_CurrentTTY->GetVGADevice()->SwapBuffers();
}

void internal_fputc(const fd_t file, const char c, bool swap) {
    internal_write(file, &c, 1);
    if (file == stdout && swap)
        g_CurrentTTY->GetVGADevice()->SwapBuffers();
}

void internal_fputs(const fd_t file, const char* str, bool swap) {
    internal_write(file, str, strlen(str));
    if (file == stdout && swap)
        g_CurrentTTY->GetVGADevice()->SwapBuffers();
}

extern "C" void dbgputc(const char c) {
    internal_write(stddebug, &c, 1);
}

extern "C" void dbgputs(const char* str) {
    internal_write(stddebug, str, strlen(str));
}

extern "C" void fputc(const fd_t file, const char c) {
    internal_fputc(file, c, true);
}

extern "C" void fputs(const fd_t file, const char* str) {
    internal_fputs(file, str, true);
}

#define PRINTF_STATE_NORMAL         0
#define PRINTF_STATE_LENGTH         1
#define PRINTF_STATE_LENGTH_SHORT   2
#define PRINTF_STATE_LENGTH_LONG    3
#define PRINTF_STATE_SPEC           4

#define PRINTF_LENGTH_DEFAULT       0
#define PRINTF_LENGTH_SHORT_SHORT   1
#define PRINTF_LENGTH_SHORT         2
#define PRINTF_LENGTH_LONG          3
#define PRINTF_LENGTH_LONG_LONG     4

const char g_HexChars[] = "0123456789abcdef";

void fprintf_unsigned(const fd_t file, uint64_t number, uint8_t radix) {
    char buffer[32];
    int pos = 0;

    // convert number to ASCII
    do 
    {
        unsigned long long rem = number % radix;
        number /= radix;
        buffer[pos++] = g_HexChars[rem];
    } while (number > 0);

    // print number in reverse order
    while (--pos >= 0)
        internal_fputc(file, buffer[pos], false);
}

void fprintf_signed(const fd_t file, int64_t number, uint8_t radix) {
    if (number < 0) {
        internal_fputc(file, '-', false);
        fprintf_unsigned(file, -number, radix);
    }
    else fprintf_unsigned(file, number, radix);
}

extern "C" void vfprintf(const fd_t file, const char* format, va_list args) {
    int state = PRINTF_STATE_NORMAL;
    int length = PRINTF_LENGTH_DEFAULT;
    uint8_t radix = 10;
    bool sign = false;
    bool number = false;
    bool gotoNextChar = true;

    uint64_t i = 0;

    char c = format[i];

    while (c != 0) {
        switch (state) {
            case PRINTF_STATE_NORMAL:
                switch (c) {
                    case '%':
                        state = PRINTF_STATE_LENGTH;
                        gotoNextChar = true;
                        break;
                    default:
                        internal_fputc(file, c, false);
                        gotoNextChar = true;
                        break;
                }
                break;

            case PRINTF_STATE_LENGTH:
                switch (c) {
                    case 'h':
                        length = PRINTF_LENGTH_SHORT;
                        state = PRINTF_STATE_LENGTH_SHORT;
                        gotoNextChar = true;
                        break;
                    case 'l':
                        length = PRINTF_LENGTH_LONG;
                        state = PRINTF_STATE_LENGTH_LONG;
                        gotoNextChar = true;
                        break;
                    default:
                        state = PRINTF_STATE_SPEC;
                        gotoNextChar = false;
                        break;
                }
                break;

            case PRINTF_STATE_LENGTH_SHORT:
                if (c == 'h') {
                    length = PRINTF_LENGTH_SHORT_SHORT;
                    gotoNextChar = true;
                }
                else gotoNextChar = false;
                state = PRINTF_STATE_SPEC;
                break;

            case PRINTF_STATE_LENGTH_LONG:
                if (c == 'h') {
                    length = PRINTF_LENGTH_LONG_LONG;
                    gotoNextChar = true;
                }
                else gotoNextChar = false;
                state = PRINTF_STATE_SPEC;
                break;

            case PRINTF_STATE_SPEC:
                switch (c) {
                    case 'c':
                        internal_fputc(file, (char)va_arg(args, int), false);
                        break;

                    case 's':   
                        internal_fputs(file, va_arg(args, const char*), false);
                        break;

                    case '%':
                        internal_fputc(file, '%', false);
                        break;

                    case 'd':
                    case 'i':
                        radix = 10; sign = true; number = true;
                        break;

                    case 'u':
                        radix = 10; sign = false; number = true;
                        break;

                    case 'X':
                    case 'x':
                    case 'p':
                        radix = 16; sign = false; number = true;
                        break;

                    case 'o':
                        radix = 8; sign = false; number = true;
                        break;

                    // ignore invalid spec
                    default:
                        break;
                }

                // actually print arg
                if (number) {
                    if (sign) {
                        switch (length) {
                        case PRINTF_LENGTH_SHORT_SHORT:
                        case PRINTF_LENGTH_SHORT:
                        case PRINTF_LENGTH_DEFAULT:     fprintf_signed(file, va_arg(args, int), radix);
                                                        break;

                        case PRINTF_LENGTH_LONG:        fprintf_signed(file, va_arg(args, long), radix);
                                                        break;

                        case PRINTF_LENGTH_LONG_LONG:   fprintf_signed(file, va_arg(args, long long), radix);
                                                        break;
                        }
                    }
                    else {
                        switch (length) {
                        case PRINTF_LENGTH_SHORT_SHORT:
                        case PRINTF_LENGTH_SHORT:
                        case PRINTF_LENGTH_DEFAULT:     fprintf_unsigned(file, va_arg(args, int), radix);
                                                        break;

                        case PRINTF_LENGTH_LONG:        fprintf_unsigned(file, va_arg(args, long), radix);
                                                        break;

                        case PRINTF_LENGTH_LONG_LONG:   fprintf_unsigned(file, va_arg(args, long long), radix);
                                                        break;
                        }
                    }
                }

                state = PRINTF_STATE_NORMAL;
                length = PRINTF_LENGTH_DEFAULT;
                radix = 10;
                sign = false;
                number = false;
                gotoNextChar = true;
                break;
        }
        if (gotoNextChar) {
            i++;
            c = format[i];
        }
    }
}

extern "C" void fprintf(const fd_t file, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(file, format, args);
    va_end(args);
    if (file == stdout)
        g_CurrentTTY->GetVGADevice()->SwapBuffers();
}

extern "C" void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    g_CurrentTTY->GetVGADevice()->SwapBuffers();
}

extern "C" void vprintf(const char* format, va_list args) {
    vfprintf(stdout, format, args);
    g_CurrentTTY->GetVGADevice()->SwapBuffers();
}

extern "C" void dbgprintf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stddebug, format, args);
    va_end(args);
}

extern "C" void dbgvprintf(const char* format, va_list args) {
    vfprintf(stddebug, format, args);
}

extern "C" size_t fwrite(const void* ptr, const size_t size, const size_t count, const fd_t file) {
    uint8_t* out = (uint8_t*)ptr;

    for (uint64_t i = 0; i < count; i+=size)
        internal_write(file, (void*)((uint64_t)out + i), size);
    if (file == stdout)
        g_CurrentTTY->GetVGADevice()->SwapBuffers();

    return count;
}

extern "C" size_t fread(void* ptr, const size_t size, const size_t count, const fd_t file) {
    for (uint64_t i = 0; i < count; i+=size)
        internal_read(file, (void*)((uint64_t)ptr + i), size);

    return count;
}

extern "C" fd_t fopen(const char* file, const char* mode) {
    if (file == nullptr || mode == nullptr)
        return -EFAULT;
    
    unsigned long i_modes;

    if (strcmp(mode, "r") == 0)
        i_modes = O_READ;
    else if (strcmp(mode, "w") == 0)
        i_modes = O_WRITE | O_CREATE;
    else if (strcmp(mode, "a") == 0)
        i_modes = O_APPEND;
    else if (strcmp(mode, "r+") == 0)
        i_modes = O_READ | O_WRITE;
    else if (strcmp(mode, "w+") == 0)
        i_modes = O_READ | O_WRITE | O_CREATE;
    else // FIXME: implement support for binary files and append/update mode
        return -EINVAL;

    return internal_open(file, i_modes);
}

extern "C" int fclose(const fd_t file) {
    return internal_close(file);
}

extern "C" int fseek(const fd_t file, long int offset, int origin) {
    if (origin != SEEK_SET)
        return -EINVAL;
    
    return internal_seek(file, offset);
}

extern "C" void rewind(const fd_t file) {
    (void)internal_seek(file, 0);
}


size_t fgetsize(const fd_t file) {
    FileDescriptor* descriptor = g_KFDManager->GetFileDescriptor(file);
    if (descriptor == nullptr)
        return 0;
    if (descriptor->GetType() != FileDescriptorType::FILE_STREAM)
        return 0;
    FileStream* stream = (FileStream*)descriptor->GetData();
    if (stream == nullptr)
        return 0;
    return stream->GetSize();
}
