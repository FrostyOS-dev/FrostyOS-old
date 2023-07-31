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

#ifndef _STDIO_HPP
#define _STDIO_HPP

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include <HAL/vfs.hpp>

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

#endif /* _STDIO_HPP */