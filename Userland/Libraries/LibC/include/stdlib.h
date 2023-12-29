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

#define RAND_MAX 4294967295


#ifndef size_t
typedef unsigned long int size_t;
#endif

typedef struct _div_t {
    int quot;
    int rem;
} div_t;

typedef struct _ldiv_t {
    long quot;
    long rem;
} ldiv_t;

typedef struct _lldiv_t {
    long long quot;
    long long rem;
} lldiv_t;


typedef unsigned long size_t;

double atof(const char* str);
int atoi(const char* str);
long atol(const char* str);
long long atoll(const char* str);
double strtod(const char* str, char** endptr);
float strtof(const char* str, char** endptr);
long strtol(const char* str, char** endptr, int base);
long double strtold(const char* str, char** endptr);
long long strtoll(const char* str, char** endptr, int base);
unsigned long strtoul(const char* str, char** endptr, int base);
unsigned long long strtoull(const char* str, char** endptr, int base);

unsigned int rand();
void srand(unsigned int);

void* calloc(size_t num, size_t size);
void free(void* ptr);
void* malloc(size_t size);
void* realloc(void* ptr, size_t size);

void abort();
int atexit(void (*func)(void));
int at_quick_exit(void (*func)(void));
void exit(int status);
char* getenv(const char* name);
void quick_exit(int status);
int system(const char* command);
void _Exit(int status);

void* bsearch(const void* key, const void* base, size_t num, size_t size, int (*compar)(const void*, const void*));
void qsort(void* base, size_t num, size_t size, int (*compar)(const void*, const void*));

int abs(int n);
long labs(long n);
long long llabs(long long n);
div_t div(int numer, int denom);
ldiv_t ldiv(long numer, long denom);
lldiv_t lldiv(long long numer, long long denom);

/* 

int mblen(const char* str, size_t n);
int mbtowc(wchar_t* pwc, const char* str, size_t n);
int wctomb(char* str, wchar_t wchar);

size_t mbstowcs(wchar_t* dst, const char* src, size_t n);
size_t wcstombs(char* dst, const wchar_t* src, size_t n);

*/

#ifdef __cplusplus
}
#endif

#endif /* _STDLIB_H */