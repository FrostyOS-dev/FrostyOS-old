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

#ifndef _STRING_H
#define _STRING_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef size_t
typedef unsigned long size_t;
#endif

void* memcpy(void* dst, const void* src, const size_t n);
void* memmove(void* dst, const void* src, const size_t n);
char* strcpy(char* dst, const char* src);
char* strncpy(char* dst, const char* src, size_t n);

char* strcat(char* dest, const char* src);
char* strncat(char* dest, const char* src, size_t num);

int memcmp(const void* s1, const void* s2, const size_t n);
int strcmp(const char* str1, const char* str2);
int strcoll(const char* str1, const char* str2);
int strncmp(const char* str1, const char* str2, size_t n);
size_t strxfrm(char* dest, const char* src, size_t n);

void* memchr(const void* ptr, int value, size_t num);
char* strchr(const char* str, int character);
size_t strcspn(const char* str1, const char* str2);
char* strpbrk(const char* str1, const char* str2);
char* strrchr(const char* str, int character);
size_t strspn(const char* str1, const char* str2);
char* strstr(const char* str1, const char* str2);
char* strtok(char* str, const char* delimiters);

void* memset(void* dst, const unsigned char value, const size_t n);
char* strerror(int errnum);
size_t strlen(const char* str);


#ifdef __cplusplus
}
#endif

#endif /* _STRING_H */
