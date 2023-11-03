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

#include <stdio.h>
#include <string.h> // just used for strlen function
#include <errno.h>
#include <stdint.h>

#include <util.h>

extern "C" {

FILE* stdin;
FILE* stdout;
FILE* stderr;
FILE* stddebug;

FILE g_stdin;
FILE g_stdout;
FILE g_stderr;
FILE g_stddebug;

FILE g_files[FOPEN_MAX];
uint8_t g_used_files; // bitmap of used files

void __stdio_init() {
    g_stdin = {0, O_READ};
    g_stdout = {1, O_WRITE};
    g_stderr = {2, O_WRITE};
    g_stddebug = {3, O_WRITE};

    stdin = &g_stdin;
    stdout = &g_stdout;
    stderr = &g_stderr;
    stddebug = &g_stddebug;

    fast_memset(g_files, 0, sizeof(FILE)); // we normally divide by 8, but since FOPEN_MAX is 8, we don't need to.
    g_used_files = 0;
}

}

long internal_read(FILE* file, void* data, size_t size) {
    if (file == nullptr) {
        __RETURN_WITH_ERRNO(-EFAULT);
    }
    __RETURN_WITH_ERRNO(read(file->descriptor, data, size));
}

long internal_write(FILE* file, const void* data, size_t size) {
    if (file == nullptr) {
        __RETURN_WITH_ERRNO(-EFAULT);
    }
    __RETURN_WITH_ERRNO(write(file->descriptor, data, size));
}

extern "C" void putc(const char c) {
    __RETURN_VOID_WITH_ERRNO(internal_write(stdout, &c, 1));
}

extern "C" void puts(const char* str) {
    __RETURN_VOID_WITH_ERRNO(internal_write(stdout, str, strlen(str)));
}

extern "C" void dbgputc(const char c) {
    __RETURN_VOID_WITH_ERRNO(internal_write(stddebug, &c, 1));
}

extern "C" void dbgputs(const char* str) {
    __RETURN_VOID_WITH_ERRNO(internal_write(stddebug, str, strlen(str)));
}

extern "C" void fputc(FILE* file, const char c) {
    __RETURN_VOID_WITH_ERRNO(internal_write(file, &c, 1));
}

extern "C" void fputs(FILE* file, const char* str) {
    __RETURN_VOID_WITH_ERRNO(internal_write(file, str, strlen(str)));
}

enum class CUSTOM_PRINTF_MODES {
    NORMAL,
    FLAGS,
    WIDTH,
    LEN,
    SPEC
};

enum class CUSTOM_PRINTF_LENGTH {
    L_CHAR,
    L_SHORT,
    L_NORMAL,
    L_LONG,
    L_LONG_LONG,
    L_INTMAX,
    L_SIZET,
    L_PTRDIFFT
};

const char g_HexCharsLOWER[] = "0123456789abcdef";
const char g_HexCharsUPPER[] = "0123456789ABCDEF";

int fprintf_uint(FILE* file, int min_length, uint64_t num, int radix, bool padding_type, bool uppercase) {
    char buffer[64] = {0}; // max size of a valid integer. Supports a full 64-bit integer in base-2
    int64_t pos = 0;
    int chars_printed = 0;

    do {
        uint64_t rem = num % radix;
        num /= radix;
        if (uppercase)
            buffer[pos++] = g_HexCharsUPPER[rem];
        else
            buffer[pos++] = g_HexCharsLOWER[rem];
    } while (num > 0);

    if (min_length > pos) {
        min_length -= pos;
        for (int i = 0; i < min_length; i++) {
            if (padding_type)
                fputc(file, '0');
            else
                fputc(file, ' ');
            chars_printed++;
        }
        for (pos--; pos >= 0; pos--) {
            fputc(file, buffer[pos]);
            chars_printed++;
        }
    }
    else {
        pos--;
        for (; pos >= 0; pos--) {
            fputc(file, buffer[pos]);
            chars_printed++;
        }
    }
    return chars_printed;
}

int fprintf_int(FILE* file, int64_t num, bool force_sign, int min_length, int radix, bool padding_type, bool uppercase) {
    int chars_printed = 0;
    if (num < 0) {
        fputc(file, '-');
        chars_printed++;
        min_length--;
        num *= -1;
    }
    else if (force_sign) {
        fputc(file, '+');
        chars_printed++;
        min_length--;
    }
    return fprintf_uint(file, min_length, num, radix, padding_type, uppercase) + chars_printed;
}

extern "C" int vfprintf(FILE* file, const char* format, va_list args) {
    int mode = (int)CUSTOM_PRINTF_MODES::NORMAL;
    int radix = 10;
    int len = (int)CUSTOM_PRINTF_LENGTH::L_NORMAL;
    bool sign = false;
    bool force_sign = false;
    bool zero_pad = false;
    bool gotoNextChar = false;
    bool number = false;
    bool upper = false;
    int symbols_printed = 0;
    uint64_t i = 0;
    char c = format[i];
    int width = 0;

    while (c != 0) {
        switch (mode) {
        case (int)CUSTOM_PRINTF_MODES::NORMAL:
            switch (c) {
            case '%':
                mode = (int)CUSTOM_PRINTF_MODES::FLAGS;
                gotoNextChar = true;
                break;
            default:
                fputc(file, c);
                gotoNextChar = true;
                symbols_printed++;
                break;
            }
            break;
        case (int)CUSTOM_PRINTF_MODES::FLAGS:
            switch (c) {
            case '+':
                force_sign = true;
                gotoNextChar = true;
                break;
            case '0':
                zero_pad = true;
                gotoNextChar = true;
                break;
            default:
                gotoNextChar = false;
                mode = (int)CUSTOM_PRINTF_MODES::WIDTH;
                break;
            }
            break;
        case (int)CUSTOM_PRINTF_MODES::WIDTH:
            if (c == '*') {
                width = va_arg(args, int);
                mode = (int)CUSTOM_PRINTF_MODES::LEN;
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
                mode = (int)CUSTOM_PRINTF_MODES::LEN;
                gotoNextChar = false;
                break;
            }
        case (int)CUSTOM_PRINTF_MODES::LEN:
            switch (c) {
            case 'h':
                if (len == (int)CUSTOM_PRINTF_LENGTH::L_SHORT)
                    len = (int)CUSTOM_PRINTF_LENGTH::L_CHAR;
                else
                    len = (int)CUSTOM_PRINTF_LENGTH::L_SHORT;
                gotoNextChar = true;
                break;
            case 'l':
                if (len == (int)CUSTOM_PRINTF_LENGTH::L_LONG)
                    len = (int)CUSTOM_PRINTF_LENGTH::L_LONG_LONG;
                else
                    len = (int)CUSTOM_PRINTF_LENGTH::L_LONG;
                gotoNextChar = true;
                break;
            case 'j':
                len = (int)CUSTOM_PRINTF_LENGTH::L_INTMAX;
                gotoNextChar = true;
                break;
            case 'z':
                len = (int)CUSTOM_PRINTF_LENGTH::L_SIZET;
                gotoNextChar = true;
                break;
            case 't':
                len = (int)CUSTOM_PRINTF_LENGTH::L_PTRDIFFT;
                gotoNextChar = true;
                break;
            default:
                mode = (int)CUSTOM_PRINTF_MODES::SPEC;
                gotoNextChar = false;
                break;
            }
            break;
        case (int)CUSTOM_PRINTF_MODES::SPEC:
            switch (c) {
            case 'd': // signed decimal
            case 'i':
                radix = 10;
                number = true;
                sign = true;
                break;
            case 'u': // unsigned decimal
                radix = 10;
                number = true;
                sign = false;
                break;
            case 'o': // unsigned octal
                radix = 8;
                number = true;
                sign = false;
                break;
            case 'p': // pointer address
            case 'x': // unsigned hex (lowercase)
                radix = 16;
                number = true;
                sign = false;
                upper = false;
                break;
            case 'X': // unsigned hex (uppercase)
                radix = 16;
                number = true;
                sign = false;
                upper = true;
                break;
            case 'c': // character
                fputc(file, va_arg(args, int /* char is promoted to int */));
                symbols_printed++;
                break;
            case 's': // string. requires max length implementation
                fputs(file, va_arg(args, const char*));
                break;
            case 'n': // send symbols_printed to va_arg
                switch (len) {
                case (int)CUSTOM_PRINTF_LENGTH::L_CHAR:
                    *(va_arg(args, signed char*)) = symbols_printed;
                    break;
                case (int)CUSTOM_PRINTF_LENGTH::L_SHORT:
                    *(va_arg(args, signed short int*)) = symbols_printed;
                    break;
                case (int)CUSTOM_PRINTF_LENGTH::L_LONG:
                    *(va_arg(args, signed long int*)) = symbols_printed;
                    break;
                case (int)CUSTOM_PRINTF_LENGTH::L_LONG_LONG:
                    *(va_arg(args, signed long long int*)) = symbols_printed;
                    break;
                case (int)CUSTOM_PRINTF_LENGTH::L_INTMAX:
                    *(va_arg(args, intmax_t*)) = symbols_printed;
                    break;
                case (int)CUSTOM_PRINTF_LENGTH::L_SIZET:
                    *(va_arg(args, size_t*)) = symbols_printed;
                    break;
                case (int)CUSTOM_PRINTF_LENGTH::L_PTRDIFFT:
                    *(va_arg(args, ptrdiff_t*)) = symbols_printed;
                    break;
                default:
                    *(va_arg(args, signed int*)) = symbols_printed;
                    break;
                }
                break;
            case '%': // print a '%'
                fputc(file, '%');
                symbols_printed++;
                break;
            default: // ignore invalid spec
                break;
            }

            // actually do the printing
            if (number) {
                if (sign) {
                    int64_t num = 0;
                    switch (len) {
                    case (int)CUSTOM_PRINTF_LENGTH::L_CHAR:
                    case (int)CUSTOM_PRINTF_LENGTH::L_SHORT:
                        num = va_arg(args, signed int);
                        break;
                    case (int)CUSTOM_PRINTF_LENGTH::L_LONG:
                        num = va_arg(args, signed long int);
                        break;
                    case (int)CUSTOM_PRINTF_LENGTH::L_LONG_LONG:
                        num = va_arg(args, signed long long int);
                        break;
                    case (int)CUSTOM_PRINTF_LENGTH::L_INTMAX:
                        num = va_arg(args, intmax_t);
                        break;
                    case (int)CUSTOM_PRINTF_LENGTH::L_SIZET:
                        num = va_arg(args, size_t);
                        break;
                    case (int)CUSTOM_PRINTF_LENGTH::L_PTRDIFFT:
                        num = va_arg(args, ptrdiff_t);
                        break;
                    default:
                        num = va_arg(args, signed int);
                        break;
                    }
                    symbols_printed += fprintf_int(file, num, force_sign, width, radix, zero_pad, upper);
                }
                else {
                    uint64_t num = 0;
                    switch (len) {
                    case (int)CUSTOM_PRINTF_LENGTH::L_CHAR:
                    case (int)CUSTOM_PRINTF_LENGTH::L_SHORT:
                        num = va_arg(args, unsigned int);
                        break;
                    case (int)CUSTOM_PRINTF_LENGTH::L_LONG:
                        num = va_arg(args, unsigned long int);
                        break;
                    case (int)CUSTOM_PRINTF_LENGTH::L_LONG_LONG:
                        num = va_arg(args, unsigned long long int);
                        break;
                    case (int)CUSTOM_PRINTF_LENGTH::L_INTMAX:
                        num = va_arg(args, uintmax_t);
                        break;
                    case (int)CUSTOM_PRINTF_LENGTH::L_SIZET:
                        num = va_arg(args, size_t);
                        break;
                    case (int)CUSTOM_PRINTF_LENGTH::L_PTRDIFFT:
                        num = va_arg(args, ptrdiff_t);
                        break;
                    default:
                        num = va_arg(args, unsigned int);
                        break;
                    }
                    symbols_printed += fprintf_uint(file, width, num, radix, zero_pad, upper);
                }
                
            }

            mode = (int)CUSTOM_PRINTF_MODES::NORMAL;
            radix = 10;
            len = (int)CUSTOM_PRINTF_LENGTH::L_NORMAL;
            sign = false;
            force_sign = false;
            zero_pad = false;
            number = false;
            upper = false;
            width = 0;
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

    return symbols_printed;
}

extern "C" int fprintf(FILE* file, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vfprintf(file, format, args);
    va_end(args);
    return ret;
}

extern "C" int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vfprintf(stdout, format, args);
    va_end(args);
    return ret;
}

extern "C" int vprintf(const char* format, va_list args) {
    return vfprintf(stdout, format, args);
}

extern "C" int dbgprintf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vfprintf(stddebug, format, args);
    va_end(args);
    return ret;
}

extern "C" int dbgvprintf(const char* format, va_list args) {
    return vfprintf(stddebug, format, args);
}

extern "C" size_t fwrite(const void* ptr, const size_t size, const size_t count, FILE* file) {
    for (uint64_t i = 0, current_count = 0; i < count; i += size, current_count++) {
        long status = internal_write(file, (void*)((uint64_t)ptr + i), size);
        if (status < 0) {
            __SET_ERRNO(status);
            return current_count;
        }
    }

    return count;
}

extern "C" size_t fread(void* ptr, const size_t size, const size_t count, FILE* file) {
    for (uint64_t i = 0, current_count = 0; i < count; i += size, current_count++) {
        long status = internal_read(file, (void*)((uint64_t)ptr + i), size);
        if (status < 0) {
            __SET_ERRNO(status);
            return current_count;
        }
    }

    return count;
}

uint8_t AllocateFile() {
    if (g_used_files < UINT8_MAX) {
        for (uint8_t i = 0; i < 8; i++) {
            if ((g_used_files & (1 << i)) == 0) {
                g_used_files |= 1 << i;
                return i;
            }
        }
    }
    return FOPEN_MAX;
}

bool FreeFile(FILE* file) {
    for (uint8_t i = 0; i < 8; i++) {
        if (&(file[i]) == file) {
            g_used_files &= ~(1 << i);
            return true;
        }
    }
    return false;
}

extern "C" FILE* fopen(const char* file, const char* mode) {
    if (file == nullptr || mode == nullptr) {
        __RETURN_NULL_WITH_ERRNO(-EFAULT);
    }
    
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
    else { // FIXME: implement support for binary files and append/update mode
        __RETURN_NULL_WITH_ERRNO(-EINVAL);
    }

    fd_t fd = open(file, i_modes);
    if (fd < 0) {
        __RETURN_NULL_WITH_ERRNO(fd);
    }

    uint8_t index = AllocateFile();
    if (index == FOPEN_MAX) {
        __RETURN_NULL_WITH_ERRNO(-ENOTSUP);
    }

    FILE* i_file = &(g_files[index]);
    i_file->descriptor = fd;
    i_file->modes = i_modes;

    __SET_ERRNO(ESUCCESS);

    return i_file;
}

extern "C" int fclose(FILE* file) {
    if (file == nullptr) {
        __RETURN_WITH_ERRNO(-EFAULT);
    }
    if (!FreeFile(file)) {
        __RETURN_WITH_ERRNO(-EBADF);
    }
    __RETURN_WITH_ERRNO(close(file->descriptor));
}

extern "C" int fseek(FILE* file, long int offset, int origin) {
    if (origin != SEEK_SET) {
        __RETURN_WITH_ERRNO(-EINVAL);
    }
    if (file == nullptr) {
        __RETURN_WITH_ERRNO(-EFAULT);
    }
    __RETURN_WITH_ERRNO(seek(file->descriptor, offset));
}

extern "C" void rewind(FILE* file) {
    (void)fseek(file, 0, SEEK_SET);
}
