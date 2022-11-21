#ifndef _KERNEL_STDIO_HPP
#define _KERNEL_STDIO_HPP

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include <HAL/vfs.hpp>

void putc(const char c);
void fputc(const fd_t file, const char c);

void puts(const char* str);
void fputs(const fd_t file, const char* str);

void printf(const char* format, ...);
void vprintf(const char* format, va_list args);
void fprintf(const fd_t file, const char* format, ...);
void vfprintf(const fd_t file, const char* format, va_list args);

#endif /* _KERNEL_STDIO_HPP */