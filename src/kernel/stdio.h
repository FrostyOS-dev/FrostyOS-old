#ifndef _KERNEL_STDIO_H
#define _KERNEL_STDIO_H

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include <HAL/vfs.h>

void putc(const char c);
void fputc(const fd_t file, const char c);

void puts(const char* str);
void fputs(const fd_t file, const char* str);

void printf(const char* format, ...);
void vprintf(const char* format, va_list args);
void fprintf(const fd_t file, const char* format, ...);
void vfprintf(const fd_t file, const char* format, va_list args);

#endif /* _KERNEL_STDIO_H */