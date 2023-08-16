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

#ifndef _STDLIB_H
#define _STDLIB_H

#ifdef __cplusplus
extern "C" {
#endif


#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

#ifndef NULL
#define NULL ((void*)0)
#endif


typedef unsigned long size_t;


int atoi(const char* str);
long atol(const char* str);


void* calloc(size_t num, size_t size);
void free(void* ptr);
void* malloc(size_t size);
void* realloc(void* ptr, size_t size);

unsigned int rand();
void srand(unsigned int);

void abort();

int atexit(void (*func)(void));

char* getenv(const char* name);

int abs(int n);

#ifdef __cplusplus
}
#endif

#endif /* _STDLIB_H */