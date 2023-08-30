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

#ifndef _STDIO_H
#define _STDIO_H

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

enum OUT_TYPES {
    VFS_STDIN            = 0,
    VFS_STDOUT           = 1,
    VFS_STDERR           = 2, // same as STDOUT for now
    VFS_DEBUG            = 3,
    VFS_DEBUG_AND_STDOUT = 4,
};

typedef uint8_t fd_t;

void putc(const char c);
inline void putchar(const char c) { putc(c); };
void fputc(const fd_t file, const char c);

void puts(const char* str);
void fputs(const fd_t file, const char* str);

void printf(const char* format, ...);
void vprintf(const char* format, va_list args);
void fprintf(const fd_t file, const char* format, ...);
void vfprintf(const fd_t file, const char* format, va_list args);

void fwrite(const void* ptr, const size_t size, const size_t count, const fd_t file);


#ifdef __cplusplus
}
#endif

#endif /* _STDIO_H */