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
#include <stdarg.h>

typedef int fd_t;

struct FILE {
    fd_t descriptor;
};

typedef struct FILE FILE;

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

#define stdin stdin
#define stdout stdout
#define stderr stderr

#define SEEK_SET 0

void putc(const char c);
inline void putchar(const char c) { putc(c); };
void fputc(FILE* file, const char c);

void puts(const char* str);
void fputs(FILE* file, const char* str);

int printf(const char* format, ...);
int vprintf(const char* format, va_list args);
int fprintf(FILE* file, const char* format, ...);
int vfprintf(FILE* file, const char* format, va_list args);
int sprintf(char* str, const char* format, ...);

size_t fwrite(const void* ptr, const size_t size, const size_t count, FILE* file);
size_t fread(void* ptr, size_t size, size_t count, FILE* stream);

FILE* fopen(const char* filename, const char* mode);
int fclose(FILE* stream);
int fflush(FILE* stream);
int fseek(FILE* stream, long int offset, int origin);
long int ftell(FILE* stream);
void setbuf(FILE* stream, char* buffer);

#endif /* _STDIO_H */