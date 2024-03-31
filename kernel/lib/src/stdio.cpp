/*
Copyright (©) 2022-2024  Frosty515

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
#include <fs/FileStream.hpp>
#include <fs/VFS.hpp>

#include <HAL/hal.hpp>

#include <file.h>

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

int64_t internal_read(fd_t file, void* data, size_t size) {
    if (data == nullptr)
        return -EFAULT;
    if (size == 0)
        return -EINVAL;
    FileDescriptor* descriptor = g_KFDManager->GetFileDescriptor(file);
    if (descriptor == nullptr)
        return -EBADF;
    size_t status = descriptor->Read((uint8_t*)data, size);
    if (status == 0) {
        switch (descriptor->GetLastError()) {
        case FileDescriptorError::INVALID_ARGUMENTS:
            return EOF;
        case FileDescriptorError::INVALID_MODE:
        case FileDescriptorError::STREAM_ERROR:
            return -EBADF;
        default: {
            PANIC("File descriptor internal error in kernel stdio.");
        }
        }
    }
    else
        return status;
}

int64_t internal_write(fd_t file, const void* data, size_t size) {
    if (data == nullptr)
        return -EFAULT;
    if (size == 0)
        return -EINVAL;
    FileDescriptor* descriptor = g_KFDManager->GetFileDescriptor(file);
    if (descriptor == nullptr)
        return -EBADF;
    size_t status = descriptor->Write((const uint8_t*)data, size);
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
        default: {
            PANIC("File descriptor internal error in kernel stdio.");
        }
        }
    }
    else
        return status;
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
                if (!g_VFS->IsValidPath(i_parent)) {
                    delete[] i_parent;
                    return -ENOENT;
                }
                parent = i_parent;
            }
            else
                parent = "/";
            bool successful = g_VFS->CreateFile({0, 0, 07777}, parent, end == nullptr ? path : child_start, 0, false, {0, 0, 00644});
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
    
    FileStream* stream = g_VFS->OpenStream({0, 0, 07777}, path, vfs_modes);
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

extern "C" int getc() {
    int c = EOF;
    long rc = internal_read(stdin, &c, 1);
    return rc == EOF ? EOF : c;
}

extern "C" int fgetc(const fd_t file) {
    int c = EOF;
    long rc = internal_read(file, &c, 1);
    return rc == EOF ? EOF : c;
}

extern "C" void putc(const char c) {
    internal_write(stdout, &c, 1);
    g_CurrentTTY->GetVGADevice()->SwapBuffers();
}

extern "C" void puts(const char* str) {
    internal_write(stdout, str, strlen(str));
    g_CurrentTTY->GetVGADevice()->SwapBuffers();
}

void internal_fputc(const fd_t file, const char c, bool swap, bool lock) {
    if (file == stdout && lock)
        g_CurrentTTY->Lock();
    internal_write(file, &c, 1);
    if (file == stdout) {
        if (swap)
            g_CurrentTTY->GetVGADevice()->SwapBuffers();
        if (lock)
            g_CurrentTTY->Unlock();
    }
}

void internal_fputs(const fd_t file, const char* str, bool swap, bool lock) {
    if (file == stdout && lock)
        g_CurrentTTY->Lock();
    internal_write(file, str, strlen(str));
    if (file == stdout && swap)
        g_CurrentTTY->GetVGADevice()->SwapBuffers();
    if (file == stdout) {
        if (swap)
            g_CurrentTTY->GetVGADevice()->SwapBuffers();
        if (lock)
            g_CurrentTTY->Unlock();
    }
}

extern "C" void dbgputc(const char c) {
    internal_write(stddebug, &c, 1);
}

extern "C" void dbgputs(const char* str) {
    internal_write(stddebug, str, strlen(str));
}

extern "C" void fputc(const fd_t file, const char c) {
    internal_fputc(file, c, true, true);
}

extern "C" void fputs(const fd_t file, const char* str) {
    internal_fputs(file, str, true, true);
}

enum class PRINTF_MODES {
    NORMAL,
    FLAGS,
    WIDTH,
    PREC,
    LEN,
    SPEC
};

enum class PRINTF_LENGTH {
    L_CHAR,
    L_SHORT,
    L_NORMAL,
    L_LONG,
    L_LONG_LONG,
    L_INTMAX,
    L_SIZET,
    L_PTRDIFFT,
    L_LDOUB
};

const char g_HexCharsLOWER[] = "0123456789abcdef";
const char g_HexCharsUPPER[] = "0123456789ABCDEF";

int fprintf_uint(fd_t file, int min_length, int min_digits, uint64_t num, int radix, bool padding_type, bool padding_orientation /*true for right, false for left*/, bool uppercase) {
    char buffer[64] = {0}; // max size of a valid integer. Supports a full 64-bit integer in base-2
    int64_t pos = 0;
    int chars_printed = 0;

    if (num == 0) {
        if (min_length > 1) {
            if (padding_orientation) {
                for (int i = 0; i < min_digits; i++) {
                    internal_fputc(file, '0', false, false);
                    min_length--;
                    chars_printed++;
                }
                if (min_digits != 0) {
                    internal_fputc(file, '0', false, false);
                    chars_printed++;
                    min_digits--;
                    min_length--;
                }
                for (int i = 0; i < min_length; i++) {
                    if (padding_type)
                        internal_fputc(file, '0', false, false);
                    else
                        internal_fputc(file, ' ', false, false);
                    chars_printed++;
                }
            }
            else {
                bool print_zero = false;
                if (min_digits != 0) {
                    print_zero = true;
                    min_digits--;
                    min_length--;
                }
                for (int i = 0; i < min_length; i++) {
                    if (padding_type || min_digits > 0) {
                        internal_fputc(file, '0', false, false);
                        min_digits--;
                    }
                    else
                        internal_fputc(file, ' ', false, false);
                    chars_printed++;
                }
                if (print_zero) {
                    internal_fputc(file, '0', false, false);
                    chars_printed++;
                }
            }
        }
        else {   
            if (min_digits != 0) {
                internal_fputc(file, '0', false, false);
                chars_printed++;
                min_digits--;
                min_length--;
            }
        }
        return chars_printed;
    }

    do {
        uint64_t rem = num % radix;
        num /= radix;
        if (uppercase)
            buffer[pos++] = g_HexCharsUPPER[rem];
        else
            buffer[pos++] = g_HexCharsLOWER[rem];
    } while (num > 0);


    if (min_length > pos) {
        if (padding_orientation) {
            min_length -= pos;
            min_digits -= pos;
            for (int i = 0; i < min_digits; i++) {
                internal_fputc(file, '0', false, false);
                min_length--;
                chars_printed++;
            }
            for (pos--; pos >= 0; pos--) {
                internal_fputc(file, buffer[pos], false, false);
                chars_printed++;
            }
            for (int i = 0; i < min_length; i++) {
                if (padding_type)
                    internal_fputc(file, '0', false, false);
                else
                    internal_fputc(file, ' ', false, false);
                chars_printed++;
            }
        }
        else {
            min_length -= pos;
            min_digits -= pos;
            for (int i = 0; i < min_length; i++) {
                if (padding_type || min_digits > 0) {
                    internal_fputc(file, '0', false, false);
                    min_digits--;
                }
                else
                    internal_fputc(file, ' ', false, false);
                chars_printed++;
            }
            for (pos--; pos >= 0; pos--) {
                internal_fputc(file, buffer[pos], false, false);
                chars_printed++;
            }
        }
    }
    else {
        pos--;
        for (; pos >= 0; pos--) {
            internal_fputc(file, buffer[pos], false, false);
            chars_printed++;
        }
    }
    return chars_printed;
}

int fprintf_int(fd_t file, int64_t num, bool force_sign, bool pad_sign, int min_length, int min_digits, int radix, bool padding_type, bool padding_orientation /*true for right, false for left*/, bool uppercase) {
    int chars_printed = 0;
    if (num < 0) {
        internal_fputc(file, '-', false, false);
        chars_printed++;
        min_length--;
        num *= -1;
    }
    else if (force_sign) {
        internal_fputc(file, '+', false, false);
        chars_printed++;
        min_length--;
    }
    else if (pad_sign) {
        internal_fputc(file, ' ', false, false);
        chars_printed++;
        min_length--;
    }
    return fprintf_uint(file, min_length, min_digits, num, radix, padding_type, padding_orientation, uppercase) + chars_printed;
}

extern "C" int vfprintf(fd_t file, const char* format, va_list args) {
    int mode = (int)PRINTF_MODES::NORMAL;
    int radix = 10;
    int len = (int)PRINTF_LENGTH::L_NORMAL;
    bool sign = false;
    bool left_justify = false;
    bool force_sign = false;
    bool add_blank = false;
    bool flags_extra = false;
    bool zero_pad = false;
    bool gotoNextChar = false;
    bool number = false;
    bool upper = false;
    bool floating = false;
    bool use_shortest = false;
    bool scientific = false;
    int symbols_printed = 0;
    uint64_t i = 0;
    char c = format[i];
    int width = 0;
    int precision = -1;

    if (file == stdout)
        g_CurrentTTY->Lock();

    while (c != 0) {
        switch (mode) {
            case (int)PRINTF_MODES::NORMAL:
                switch (c) {
                    case '%':
                        mode = (int)PRINTF_MODES::FLAGS;
                        gotoNextChar = true;
                        break;
                    default:
                        internal_fputc(file, c, false, false);
                        gotoNextChar = true;
                        symbols_printed++;
                        break;
                }
                break;
            case (int)PRINTF_MODES::FLAGS:
                switch (c) {
                    case '-':
                        left_justify = true;
                        gotoNextChar = true;
                        break;
                    case '+':
                        force_sign = true;
                        gotoNextChar = true;
                        break;
                    case ' ':
                        add_blank = true;
                        gotoNextChar = true;
                        break;
                    case '#':
                        flags_extra = true;
                        gotoNextChar = true;
                        break;
                    case '0':
                        zero_pad = true;
                        gotoNextChar = true;
                        break;
                    default:
                        gotoNextChar = false;
                        mode = (int)PRINTF_MODES::WIDTH;
                        break;
                }
                break;
            case (int)PRINTF_MODES::WIDTH:
                if (c == '*') {
                    width = va_arg(args, int);
                    mode = (int)PRINTF_MODES::LEN;
                    gotoNextChar = true;
                    break;
                }
                else if (c >= '0' && c <= '9') {
                    width *= 10;
                    width += c - '0';
                    gotoNextChar = true;
                    break;
                }
                else {
                    mode = (int)PRINTF_MODES::LEN;
                    gotoNextChar = false;
                    break;
                }
            case (int)PRINTF_MODES::PREC:
                if (c == '*') {
                    precision = va_arg(args, int);
                    mode = (int)PRINTF_MODES::LEN;
                    gotoNextChar = true;
                    break;
                }
                else if (c >= '0' && c <= '9') {
                    precision *= 10;
                    precision += c - '0';
                    gotoNextChar = true;
                    break;
                }
                else {
                    mode = (int)PRINTF_MODES::LEN;
                    gotoNextChar = false;
                    break;
                }
            case (int)PRINTF_MODES::LEN:
                switch (c) {
                    case '.':
                        if (precision == -1) { // prevent against invalid position of precision
                            mode = (int)PRINTF_MODES::PREC;
                            gotoNextChar = true;
                            precision = 0;
                            break;
                        }
                        assert(false);
                        break;
                    case 'h':
                        if (len == (int)PRINTF_LENGTH::L_SHORT)
                            len = (int)PRINTF_LENGTH::L_CHAR;
                        else
                            len = (int)PRINTF_LENGTH::L_SHORT;
                        gotoNextChar = true;
                        break;
                    case 'l':
                        if (len == (int)PRINTF_LENGTH::L_LONG)
                            len = (int)PRINTF_LENGTH::L_LONG_LONG;
                        else
                            len = (int)PRINTF_LENGTH::L_LONG;
                        gotoNextChar = true;
                        break;
                    case 'j':
                        len = (int)PRINTF_LENGTH::L_INTMAX;
                        gotoNextChar = true;
                        break;
                    case 'z':
                        len = (int)PRINTF_LENGTH::L_SIZET;
                        gotoNextChar = true;
                        break;
                    case 't':
                        len = (int)PRINTF_LENGTH::L_PTRDIFFT;
                        gotoNextChar = true;
                        break;
                    case 'L':
                        len = (int)PRINTF_LENGTH::L_LDOUB;
                        gotoNextChar = true;
                        break;
                    default:
                        mode = (int)PRINTF_MODES::SPEC;
                        gotoNextChar = false;
                        break;
                }
                break;
            case (int)PRINTF_MODES::SPEC:
                switch (c) {
                    case 'd': // signed decimal
                    case 'i':
                        radix = 10;
                        number = true;
                        sign = true;
                        floating = false;
                        break;
                    case 'u': // unsigned decimal
                        radix = 10;
                        number = true;
                        sign = false;
                        floating = false;
                        break;
                    case 'o': // unsigned octal
                        radix = 8;
                        number = true;
                        sign = false;
                        floating = false;
                        break;
                    case 'p': // pointer address
                        if (len == (int)PRINTF_LENGTH::L_NORMAL)
                            len = (int)PRINTF_LENGTH::L_LONG;
                        radix = 16;
                        number = true;
                        sign = false;
                        upper = false;
                        floating = false;
                        break;
                    case 'x': // unsigned hex (lowercase)
                        radix = 16;
                        number = true;
                        sign = false;
                        upper = false;
                        floating = false;
                        break;
                    case 'X': // unsigned hex (uppercase)
                        radix = 16;
                        number = true;
                        sign = false;
                        upper = true;
                        floating = false;
                        break;
                    case 'f': // decimal floating point (lowercase)
                        radix = 10;
                        number = true;
                        sign = true;
                        upper = false;
                        floating = true;
                        break;
                    case 'F': // decimal floating point (uppercase)
                        radix = 10;
                        number = true;
                        sign = true;
                        upper = true;
                        floating = true;
                        break;
                    case 'e': // scientific notation (lowercase)
                        radix = 10;
                        number = true;
                        sign = true;
                        floating = true;
                        upper = false;
                        scientific = true;
                        break;
                    case 'E': // scientific notation (uppercase)
                        radix = 10;
                        number = true;
                        sign = true;
                        floating = true;
                        upper = true;
                        scientific = true;
                        break;
                    case 'g': // choose 'f' or 'e', whatever is shorter
                        radix = 10;
                        number = true;
                        floating = true;
                        upper = false;
                        use_shortest = true;
                        sign = true;
                        break;
                    case 'G': // choose 'F' or 'E', whatever is shorter
                        radix = 10;
                        number = true;
                        floating = true;
                        upper = true;
                        use_shortest = true;
                        sign = true;
                        break;
                    case 'a': // Hex floating point (lowercase)
                        radix = 16;
                        upper = false;
                        floating = true;
                        sign = true;
                        number = true;
                        break;
                    case 'A': // Hex floating point (uppercase)
                        radix = 16;
                        upper = true;
                        floating = true;
                        sign = true;
                        number = true;
                        break;
                    case 'c': // character
                        internal_fputc(file, va_arg(args, int /* char is promoted to int */), false, false);
                        symbols_printed++;
                        break;
                    case 's': // string.
                        {
                            const char* str = va_arg(args, const char*);
                            uint64_t j = 0;
                            char c2 = str[j];
                            while (c2 != 0) {
                                if (precision > 0 && j >= (unsigned int)precision)
                                    break;
                                internal_fputc(file, c2, false, false);
                                symbols_printed++;
                                j++;
                                c2 = str[j];
                                width--;
                            }
                            for (int j = 0; j < width; j++) {
                                internal_fputc(file, ' ', false, false);
                                symbols_printed++;
                            }
                            break;
                        }
                    case 'n': // send symbols_printed to va_arg
                        switch (len) {
                            case (int)PRINTF_LENGTH::L_CHAR:
                                *(va_arg(args, signed char*)) = symbols_printed;
                                break;
                            case (int)PRINTF_LENGTH::L_SHORT:
                                *(va_arg(args, signed short int*)) = symbols_printed;
                                break;
                            case (int)PRINTF_LENGTH::L_LONG:
                                *(va_arg(args, signed long int*)) = symbols_printed;
                                break;
                            case (int)PRINTF_LENGTH::L_LONG_LONG:
                                *(va_arg(args, signed long long int*)) = symbols_printed;
                                break;
                            case (int)PRINTF_LENGTH::L_INTMAX:
                                *(va_arg(args, intmax_t*)) = symbols_printed;
                                break;
                            case (int)PRINTF_LENGTH::L_SIZET:
                                *(va_arg(args, size_t*)) = symbols_printed;
                                break;
                            case (int)PRINTF_LENGTH::L_PTRDIFFT:
                                *(va_arg(args, ptrdiff_t*)) = symbols_printed;
                                break;
                            default:
                                *(va_arg(args, signed int*)) = symbols_printed;
                                break;
                        }
                        break;
                    case '%': // print a '%'
                        internal_fputc(file, '%', false, false);
                        symbols_printed++;
                        break;
                    default: // ignore invalid spec
                        break;
                }

                if (flags_extra) {
                    switch (radix) {
                    case 16:
                        internal_fputc(file, '0', false, false);
                        internal_fputc(file, upper ? 'X' : 'x', false, false);
                        symbols_printed += 2;
                        width -= 2;
                        if (width < 0)
                            width = 0;
                        break;
                    case 8:
                        internal_fputc(file, '0', false, false);
                        symbols_printed++;
                        width--;
                        if (width < 0)
                            width = 0;
                        break;
                    default:
                        break;
                    }
                }


                // actually do the printing
                if (number) {
                    if (floating) {
                        (void)use_shortest;
                        (void)scientific;
                        assert(false);
                    }
                    else {
                        if (sign) {
                            int64_t num = 0;
                            switch (len) {
                                case (int)PRINTF_LENGTH::L_CHAR:
                                case (int)PRINTF_LENGTH::L_SHORT:
                                    num = va_arg(args, signed int);
                                    break;
                                case (int)PRINTF_LENGTH::L_LONG:
                                    num = va_arg(args, signed long int);
                                    break;
                                case (int)PRINTF_LENGTH::L_LONG_LONG:
                                    num = va_arg(args, signed long long int);
                                    break;
                                case (int)PRINTF_LENGTH::L_INTMAX:
                                    num = va_arg(args, intmax_t);
                                    break;
                                case (int)PRINTF_LENGTH::L_SIZET:
                                    num = va_arg(args, size_t);
                                    break;
                                case (int)PRINTF_LENGTH::L_PTRDIFFT:
                                    num = va_arg(args, ptrdiff_t);
                                    break;
                                default:
                                    num = va_arg(args, signed int);
                                    break;
                            }
                            symbols_printed += fprintf_int(file, num, force_sign, add_blank, max(width, precision), precision, radix, zero_pad, left_justify, upper);
                        }
                        else {
                            uint64_t num = 0;
                            switch (len) {
                                case (int)PRINTF_LENGTH::L_CHAR:
                                case (int)PRINTF_LENGTH::L_SHORT:
                                    num = va_arg(args, unsigned int);
                                    break;
                                case (int)PRINTF_LENGTH::L_LONG:
                                    num = va_arg(args, unsigned long int);
                                    break;
                                case (int)PRINTF_LENGTH::L_LONG_LONG:
                                    num = va_arg(args, unsigned long long int);
                                    break;
                                case (int)PRINTF_LENGTH::L_INTMAX:
                                    num = va_arg(args, uintmax_t);
                                    break;
                                case (int)PRINTF_LENGTH::L_SIZET:
                                    num = va_arg(args, size_t);
                                    break;
                                case (int)PRINTF_LENGTH::L_PTRDIFFT:
                                    num = va_arg(args, ptrdiff_t);
                                    break;
                                default:
                                    num = va_arg(args, unsigned int);
                                    break;
                            }
                            symbols_printed += fprintf_uint(file, max(width, precision), precision, num, radix, zero_pad, left_justify, upper);
                        }
                    }
                }

                mode = (int)PRINTF_MODES::NORMAL;
                radix = 10;
                len = (int)PRINTF_LENGTH::L_NORMAL;
                sign = false;
                left_justify = false;
                force_sign = false;
                add_blank = false;
                flags_extra = false;
                zero_pad = false;
                number = false;
                upper = false;
                floating = false;
                use_shortest = false;
                scientific = false;
                width = 0;
                precision = -1;
                gotoNextChar = true;
                break;
            default:
                if (file == stdout)
                    g_CurrentTTY->Unlock();
                return -1; // invalid mode
        }
        if (gotoNextChar) {
            i++;
            c = format[i];
        }
    }


    if (file == stdout)
        g_CurrentTTY->Unlock();

    return symbols_printed;
}

void snputc(char* s, size_t max_size, size_t* current_size, char c) {
    if (*current_size < max_size) {
        s[*current_size] = c;
        *current_size += 1;
    }
}

int snprintf_uint(char* s, size_t max_size, size_t* current_size, int min_length, int min_digits, uint64_t num, int radix, bool padding_type, bool padding_orientation /*true for right, false for left*/, bool uppercase) {
    char buffer[64] = {0}; // max size of a valid integer. Supports a full 64-bit integer in base-2
    int64_t pos = 0;
    int chars_printed = 0;

    if (num == 0) {
        if (min_length > 1) {
            if (padding_orientation) {
                for (int i = 0; i < min_digits; i++) {
                    snputc(s, max_size, current_size, '0');
                    min_length--;
                    chars_printed++;
                }
                if (min_digits != 0) {
                    snputc(s, max_size, current_size, '0');
                    chars_printed++;
                    min_digits--;
                    min_length--;
                }
                for (int i = 0; i < min_length; i++) {
                    if (padding_type)
                        snputc(s, max_size, current_size, '0');
                    else
                        snputc(s, max_size, current_size, ' ');
                    chars_printed++;
                }
            }
            else {
                bool print_zero = false;
                if (min_digits != 0) {
                    print_zero = true;
                    min_digits--;
                    min_length--;
                }
                for (int i = 0; i < min_length; i++) {
                    if (padding_type || min_digits > 0) {
                        snputc(s, max_size, current_size, '0');
                        min_digits--;
                    }
                    else
                        snputc(s, max_size, current_size, ' ');
                    chars_printed++;
                }
                if (print_zero) {
                    snputc(s, max_size, current_size, '0');
                    chars_printed++;
                }
            }
        }
        else {   
            if (min_digits != 0) {
                snputc(s, max_size, current_size, '0');
                chars_printed++;
                min_digits--;
                min_length--;
            }
        }
        return chars_printed;
    }

    do {
        uint64_t rem = num % radix;
        num /= radix;
        if (uppercase)
            buffer[pos++] = g_HexCharsUPPER[rem];
        else
            buffer[pos++] = g_HexCharsLOWER[rem];
    } while (num > 0);


    if (min_length > pos) {
        if (padding_orientation) {
            min_length -= pos;
            min_digits -= pos;
            for (int i = 0; i < min_digits; i++) {
                snputc(s, max_size, current_size, '0');
                min_length--;
                chars_printed++;
            }
            for (pos--; pos >= 0; pos--) {
                snputc(s, max_size, current_size, buffer[pos]);
                chars_printed++;
            }
            for (int i = 0; i < min_length; i++) {
                if (padding_type)
                    snputc(s, max_size, current_size, '0');
                else
                    snputc(s, max_size, current_size, ' ');
                chars_printed++;
            }
        }
        else {
            min_length -= pos;
            min_digits -= pos;
            for (int i = 0; i < min_length; i++) {
                if (padding_type || min_digits > 0) {
                    snputc(s, max_size, current_size, '0');
                    min_digits--;
                }
                else
                    snputc(s, max_size, current_size, ' ');
                chars_printed++;
            }
            for (pos--; pos >= 0; pos--) {
                snputc(s, max_size, current_size, buffer[pos]);
                chars_printed++;
            }
        }
    }
    else {
        pos--;
        for (; pos >= 0; pos--) {
            snputc(s, max_size, current_size, buffer[pos]);
            chars_printed++;
        }
    }
    return chars_printed;
}

int snprintf_int(char* s, size_t max_size, size_t* current_size, int64_t num, bool force_sign, bool pad_sign, int min_length, int min_digits, int radix, bool padding_type, bool padding_orientation /*true for right, false for left*/, bool uppercase) {
    int chars_printed = 0;
    if (num < 0) {
        snputc(s, max_size, current_size, '-');
        chars_printed++;
        min_length--;
        num *= -1;
    }
    else if (force_sign) {
        snputc(s, max_size, current_size, '+');
        chars_printed++;
        min_length--;
    }
    else if (pad_sign) {
        snputc(s, max_size, current_size, ' ');
        chars_printed++;
        min_length--;
    }
    return snprintf_uint(s, max_size, current_size, min_length, min_digits, num, radix, padding_type, padding_orientation, uppercase) + chars_printed;
}

extern "C" int vsnprintf(char* s, size_t n, const char* format, va_list args) {
    n--; // for the null terminating character

    int mode = (int)PRINTF_MODES::NORMAL;
    int radix = 10;
    int len = (int)PRINTF_LENGTH::L_NORMAL;
    bool sign = false;
    bool left_justify = false;
    bool force_sign = false;
    bool add_blank = false;
    bool flags_extra = false;
    bool zero_pad = false;
    bool gotoNextChar = false;
    bool number = false;
    bool upper = false;
    bool floating = false;
    bool use_shortest = false;
    bool scientific = false;
    int symbols_printed = 0;
    uint64_t i = 0;
    char c = format[i];
    int width = 0;
    int precision = -1;

    size_t current_size = 0;

    while (c != 0) {
        switch (mode) {
            case (int)PRINTF_MODES::NORMAL:
                switch (c) {
                    case '%':
                        mode = (int)PRINTF_MODES::FLAGS;
                        gotoNextChar = true;
                        break;
                    default:
                        snputc(s, n, &current_size, c);
                        gotoNextChar = true;
                        symbols_printed++;
                        break;
                }
                break;
            case (int)PRINTF_MODES::FLAGS:
                switch (c) {
                    case '-':
                        left_justify = true;
                        gotoNextChar = true;
                        break;
                    case '+':
                        force_sign = true;
                        gotoNextChar = true;
                        break;
                    case ' ':
                        add_blank = true;
                        gotoNextChar = true;
                        break;
                    case '#':
                        flags_extra = true;
                        gotoNextChar = true;
                        break;
                    case '0':
                        zero_pad = true;
                        gotoNextChar = true;
                        break;
                    default:
                        gotoNextChar = false;
                        mode = (int)PRINTF_MODES::WIDTH;
                        break;
                }
                break;
            case (int)PRINTF_MODES::WIDTH:
                if (c == '*') {
                    width = va_arg(args, int);
                    mode = (int)PRINTF_MODES::LEN;
                    gotoNextChar = true;
                    break;
                }
                else if (c >= '0' && c <= '9') {
                    width *= 10;
                    width += c - '0';
                    gotoNextChar = true;
                    break;
                }
                else {
                    mode = (int)PRINTF_MODES::LEN;
                    gotoNextChar = false;
                    break;
                }
            case (int)PRINTF_MODES::PREC:
                if (c == '*') {
                    precision = va_arg(args, int);
                    mode = (int)PRINTF_MODES::LEN;
                    gotoNextChar = true;
                    break;
                }
                else if (c >= '0' && c <= '9') {
                    precision *= 10;
                    precision += c - '0';
                    gotoNextChar = true;
                    break;
                }
                else {
                    mode = (int)PRINTF_MODES::LEN;
                    gotoNextChar = false;
                    break;
                }
            case (int)PRINTF_MODES::LEN:
                switch (c) {
                    case '.':
                        if (precision == -1) { // prevent against invalid position of precision
                            mode = (int)PRINTF_MODES::PREC;
                            gotoNextChar = true;
                            precision = 0;
                            break;
                        }
                        assert(false);
                        break;
                    case 'h':
                        if (len == (int)PRINTF_LENGTH::L_SHORT)
                            len = (int)PRINTF_LENGTH::L_CHAR;
                        else
                            len = (int)PRINTF_LENGTH::L_SHORT;
                        gotoNextChar = true;
                        break;
                    case 'l':
                        if (len == (int)PRINTF_LENGTH::L_LONG)
                            len = (int)PRINTF_LENGTH::L_LONG_LONG;
                        else
                            len = (int)PRINTF_LENGTH::L_LONG;
                        gotoNextChar = true;
                        break;
                    case 'j':
                        len = (int)PRINTF_LENGTH::L_INTMAX;
                        gotoNextChar = true;
                        break;
                    case 'z':
                        len = (int)PRINTF_LENGTH::L_SIZET;
                        gotoNextChar = true;
                        break;
                    case 't':
                        len = (int)PRINTF_LENGTH::L_PTRDIFFT;
                        gotoNextChar = true;
                        break;
                    case 'L':
                        len = (int)PRINTF_LENGTH::L_LDOUB;
                        gotoNextChar = true;
                        break;
                    default:
                        mode = (int)PRINTF_MODES::SPEC;
                        gotoNextChar = false;
                        break;
                }
                break;
            case (int)PRINTF_MODES::SPEC:
                switch (c) {
                    case 'd': // signed decimal
                    case 'i':
                        radix = 10;
                        number = true;
                        sign = true;
                        floating = false;
                        break;
                    case 'u': // unsigned decimal
                        radix = 10;
                        number = true;
                        sign = false;
                        floating = false;
                        break;
                    case 'o': // unsigned octal
                        radix = 8;
                        number = true;
                        sign = false;
                        floating = false;
                        break;
                    case 'p': // pointer address
                        if (len == (int)PRINTF_LENGTH::L_NORMAL)
                            len = (int)PRINTF_LENGTH::L_LONG;
                        radix = 16;
                        number = true;
                        sign = false;
                        upper = false;
                        floating = false;
                        break;
                    case 'x': // unsigned hex (lowercase)
                        radix = 16;
                        number = true;
                        sign = false;
                        upper = false;
                        floating = false;
                        break;
                    case 'X': // unsigned hex (uppercase)
                        radix = 16;
                        number = true;
                        sign = false;
                        upper = true;
                        floating = false;
                        break;
                    case 'f': // decimal floating point (lowercase)
                        radix = 10;
                        number = true;
                        sign = true;
                        upper = false;
                        floating = true;
                        break;
                    case 'F': // decimal floating point (uppercase)
                        radix = 10;
                        number = true;
                        sign = true;
                        upper = true;
                        floating = true;
                        break;
                    case 'e': // scientific notation (lowercase)
                        radix = 10;
                        number = true;
                        sign = true;
                        floating = true;
                        upper = false;
                        scientific = true;
                        break;
                    case 'E': // scientific notation (uppercase)
                        radix = 10;
                        number = true;
                        sign = true;
                        floating = true;
                        upper = true;
                        scientific = true;
                        break;
                    case 'g': // choose 'f' or 'e', whatever is shorter
                        radix = 10;
                        number = true;
                        floating = true;
                        upper = false;
                        use_shortest = true;
                        sign = true;
                        break;
                    case 'G': // choose 'F' or 'E', whatever is shorter
                        radix = 10;
                        number = true;
                        floating = true;
                        upper = true;
                        use_shortest = true;
                        sign = true;
                        break;
                    case 'a': // Hex floating point (lowercase)
                        radix = 16;
                        upper = false;
                        floating = true;
                        sign = true;
                        number = true;
                        break;
                    case 'A': // Hex floating point (uppercase)
                        radix = 16;
                        upper = true;
                        floating = true;
                        sign = true;
                        number = true;
                        break;
                    case 'c': // character
                        snputc(s, n, &current_size, va_arg(args, int /* char is promoted to int */));
                        symbols_printed++;
                        break;
                    case 's': // string.
                        {
                            const char* str = va_arg(args, const char*);
                            uint64_t j = 0;
                            char c2 = str[j];
                            while (c2 != 0) {
                                if (precision > 0 && j >= (unsigned int)precision)
                                    break;
                                snputc(s, n, &current_size, c2);
                                symbols_printed++;
                                j++;
                                c2 = str[j];
                                width--;
                            }
                            for (int j = 0; j < width; j++) {
                                snputc(s, n, &current_size, ' ');
                                symbols_printed++;
                            }
                            break;
                        }
                    case 'n': // send symbols_printed to va_arg
                        switch (len) {
                            case (int)PRINTF_LENGTH::L_CHAR:
                                *(va_arg(args, signed char*)) = symbols_printed;
                                break;
                            case (int)PRINTF_LENGTH::L_SHORT:
                                *(va_arg(args, signed short int*)) = symbols_printed;
                                break;
                            case (int)PRINTF_LENGTH::L_LONG:
                                *(va_arg(args, signed long int*)) = symbols_printed;
                                break;
                            case (int)PRINTF_LENGTH::L_LONG_LONG:
                                *(va_arg(args, signed long long int*)) = symbols_printed;
                                break;
                            case (int)PRINTF_LENGTH::L_INTMAX:
                                *(va_arg(args, intmax_t*)) = symbols_printed;
                                break;
                            case (int)PRINTF_LENGTH::L_SIZET:
                                *(va_arg(args, size_t*)) = symbols_printed;
                                break;
                            case (int)PRINTF_LENGTH::L_PTRDIFFT:
                                *(va_arg(args, ptrdiff_t*)) = symbols_printed;
                                break;
                            default:
                                *(va_arg(args, signed int*)) = symbols_printed;
                                break;
                        }
                        break;
                    case '%': // print a '%'
                        snputc(s, n, &current_size, '%');
                        symbols_printed++;
                        break;
                    default: // ignore invalid spec
                        break;
                }

                if (flags_extra) {
                    switch (radix) {
                    case 16:
                        snputc(s, n, &current_size, '0');
                        snputc(s, n, &current_size, upper ? 'X' : 'x');
                        symbols_printed += 2;
                        width -= 2;
                        if (width < 0)
                            width = 0;
                        break;
                    case 8:
                        snputc(s, n, &current_size, '0');
                        symbols_printed++;
                        width--;
                        if (width < 0)
                            width = 0;
                        break;
                    default:
                        break;
                    }
                }


                // actually do the printing
                if (number) {
                    if (floating) {
                        (void)use_shortest;
                        (void)scientific;
                        assert(false);
                    }
                    else {
                        if (sign) {
                            int64_t num = 0;
                            switch (len) {
                                case (int)PRINTF_LENGTH::L_CHAR:
                                case (int)PRINTF_LENGTH::L_SHORT:
                                    num = va_arg(args, signed int);
                                    break;
                                case (int)PRINTF_LENGTH::L_LONG:
                                    num = va_arg(args, signed long int);
                                    break;
                                case (int)PRINTF_LENGTH::L_LONG_LONG:
                                    num = va_arg(args, signed long long int);
                                    break;
                                case (int)PRINTF_LENGTH::L_INTMAX:
                                    num = va_arg(args, intmax_t);
                                    break;
                                case (int)PRINTF_LENGTH::L_SIZET:
                                    num = va_arg(args, size_t);
                                    break;
                                case (int)PRINTF_LENGTH::L_PTRDIFFT:
                                    num = va_arg(args, ptrdiff_t);
                                    break;
                                default:
                                    num = va_arg(args, signed int);
                                    break;
                            }
                            symbols_printed += snprintf_int(s, n, &current_size, num, force_sign, add_blank, max(width, precision), precision, radix, zero_pad, left_justify, upper);
                        }
                        else {
                            uint64_t num = 0;
                            switch (len) {
                                case (int)PRINTF_LENGTH::L_CHAR:
                                case (int)PRINTF_LENGTH::L_SHORT:
                                    num = va_arg(args, unsigned int);
                                    break;
                                case (int)PRINTF_LENGTH::L_LONG:
                                    num = va_arg(args, unsigned long int);
                                    break;
                                case (int)PRINTF_LENGTH::L_LONG_LONG:
                                    num = va_arg(args, unsigned long long int);
                                    break;
                                case (int)PRINTF_LENGTH::L_INTMAX:
                                    num = va_arg(args, uintmax_t);
                                    break;
                                case (int)PRINTF_LENGTH::L_SIZET:
                                    num = va_arg(args, size_t);
                                    break;
                                case (int)PRINTF_LENGTH::L_PTRDIFFT:
                                    num = va_arg(args, ptrdiff_t);
                                    break;
                                default:
                                    num = va_arg(args, unsigned int);
                                    break;
                            }
                            symbols_printed += snprintf_uint(s, n, &current_size, max(width, precision), precision, num, radix, zero_pad, left_justify, upper);
                        }
                    }
                }

                mode = (int)PRINTF_MODES::NORMAL;
                radix = 10;
                len = (int)PRINTF_LENGTH::L_NORMAL;
                sign = false;
                left_justify = false;
                force_sign = false;
                add_blank = false;
                flags_extra = false;
                zero_pad = false;
                number = false;
                upper = false;
                floating = false;
                use_shortest = false;
                scientific = false;
                width = 0;
                precision = -1;
                gotoNextChar = true;
                break;
            default:
                return -1; // invalid mode
        }
        if (gotoNextChar) {
            i++;
            c = format[i];
        }
    }

    snputc(s, SIZE_MAX, &current_size, 0); // max_length is irrelevant for this as we know that there is always room for a null termination.

    return symbols_printed;
}

extern "C" int fprintf(const fd_t file, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int rc = vfprintf(file, format, args);
    va_end(args);
    if (file == stdout)
        g_CurrentTTY->GetVGADevice()->SwapBuffers();
    return rc;
}

extern "C" int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int rc = vfprintf(stdout, format, args);
    va_end(args);
    g_CurrentTTY->GetVGADevice()->SwapBuffers();
    return rc;
}

extern "C" int vprintf(const char* format, va_list args) {
    int rc = vfprintf(stdout, format, args);
    g_CurrentTTY->GetVGADevice()->SwapBuffers();
    return rc;
}

extern "C" int vsprintf(char* s, const char* format, va_list args) {
    return vsnprintf(s, SIZE_MAX, format, args);
}

extern "C" int sprintf(char* s, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int rc = vsnprintf(s, SIZE_MAX, format, args);
    va_end(args);
    return rc;
}

extern "C" int snprintf(char* s, size_t n, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int rc = vsnprintf(s, n, format, args);
    va_end(args);
    return rc;
}

extern "C" int dbgprintf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int rc = vfprintf(stddebug, format, args);
    va_end(args);
    return rc;
}

extern "C" int dbgvprintf(const char* format, va_list args) {
    return vfprintf(stddebug, format, args);
}

extern "C" size_t fwrite(const void* ptr, const size_t size, const size_t count, const fd_t file) {
    uint8_t* out = (uint8_t*)ptr;

    size_t blocks_written = 0;
    for (uint64_t i = 0; i < count; i++) {
        if (internal_write(file, (void*)((uint64_t)out + (i * size)), size) > 0)
            blocks_written++;
        else
            break;
    }
    if (file == stdout && blocks_written > 0)
        g_CurrentTTY->GetVGADevice()->SwapBuffers();

    return blocks_written;
}

extern "C" size_t fread(void* ptr, const size_t size, const size_t count, const fd_t file) {
    size_t blocks_read = 0;
    for (uint64_t i = 0; i < count; i++) {
        if (internal_read(file, (void*)((uint64_t)ptr + (i * size)), size) > 0)
            blocks_read++;
        else
            break;
    }

    return blocks_read;
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
