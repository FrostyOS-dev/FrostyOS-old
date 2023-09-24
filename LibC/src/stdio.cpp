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

void fprintf_unsigned(FILE* file, uint64_t number, uint8_t radix) {
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
        fputc(file, buffer[pos]);
}

void fprintf_signed(FILE* file, int64_t number, uint8_t radix) {
    if (number < 0) {
        fputc(file, '-');
        fprintf_unsigned(file, -number, radix);
    }
    else fprintf_unsigned(file, number, radix);
}

extern "C" int vfprintf(FILE* file, const char* format, va_list args) {
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
                        fputc(file, c);
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
                        fputc(file, (char)va_arg(args, int));
                        break;

                    case 's':   
                        fputs(file, va_arg(args, const char*));
                        break;

                    case '%':
                        fputc(file, '%');
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
    return 0; // FIXME: return proper value
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
