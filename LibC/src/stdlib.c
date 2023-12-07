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

#include <stdlib.h>

#include <kernel/syscall.h>

int atoi(const char* str) {
    int value = 0;
    unsigned char is_negative = 0;
    int i = 0;
    if (str[0] == '-') {
        is_negative = 1;
        i++;
    }
    while (str[i] != 0) {
        value += str[i] - '0';
        value *= 10;
        i++;
    }
    if (is_negative == 1) {
        value *= -1;
    }
    return value;
}

long atol(const char* str) {
    long value = 0;
    unsigned char is_negative = 0;
    int i = 0;
    if (str[0] == '-') {
        is_negative = 1;
        i++;
    }
    while (str[i] != 0) {
        value += str[i] - '0';
        value *= 10;
        i++;
    }
    if (is_negative == 1) {
        value *= -1;
    }
    return value;
}

long long atoll(const char* str) {
    return atol(str);
}

long strtol(const char* str, char** endptr, int base) {
    long value = 0;
    unsigned char is_negative = 0;
    int i = 0;
    if (str[0] == '-') {
        is_negative = 1;
        i++;
    }
    while (str[i] != 0) {
        if (base == 16 && str[i] >= 'a' && str[i] <= 'f') {
            value *= base;
            value += str[i] - 'a' + 10;
        }
        else if (base == 16 && str[i] >= 'A' && str[i] <= 'F') {
            value *= base;
            value += str[i] - 'A' + 10;
        }
        else if (base == 16 && str[i] >= '0' && str[i] <= '9') {
            value *= base;
            value += str[i] - '0';
        }
        else if (base == 10 && str[i] >= '0' && str[i] <= '9') {
            value *= base;
            value += str[i] - '0';
        }
        else if (base == 8 && str[i] >= '0' && str[i] <= '7') {
            value *= base;
            value += str[i] - '0';
        }
        else if (base == 2 && str[i] >= '0' && str[i] <= '1') {
            value *= base;
            value += str[i] - '0';
        }
        else {
            if (endptr != NULL)
                *endptr = (char*)&str[i];
            break;
        }
        i++;
    }
    if (is_negative == 1) {
        value *= -1;
    }
    return value;
}

long long strtoll(const char* str, char** endptr, int base) {
    return strtol(str, endptr, base);
}

unsigned long strtoul(const char* str, char** endptr, int base) {
    unsigned long value = 0;
    int i = 0;
    while (str[i] != 0) {
        if (base == 16 && str[i] >= 'a' && str[i] <= 'f') {
            value *= base;
            value += str[i] - 'a' + 10;
        }
        else if (base == 16 && str[i] >= 'A' && str[i] <= 'F') {
            value *= base;
            value += str[i] - 'A' + 10;
        }
        else if (base == 16 && str[i] >= '0' && str[i] <= '9') {
            value *= base;
            value += str[i] - '0';
        }
        else if (base == 10 && str[i] >= '0' && str[i] <= '9') {
            value *= base;
            value += str[i] - '0';
        }
        else if (base == 8 && str[i] >= '0' && str[i] <= '7') {
            value *= base;
            value += str[i] - '0';
        }
        else if (base == 2 && str[i] >= '0' && str[i] <= '1') {
            value *= base;
            value += str[i] - '0';
        }
        else {
            if (endptr != NULL)
                *endptr = (char*)&str[i];
            break;
        }
        i++;
    }
    return value;
}

unsigned long long strtoull(const char* str, char** endptr, int base) {
    return strtoul(str, endptr, base);
}

static unsigned int rseed = 1;

void srand(unsigned int s) {
    rseed = s;
}

unsigned int rand() {
    static unsigned int x = 123456789;
    static unsigned int y = 362436069;
    static unsigned int z = 521288629;
    static unsigned int w = 88675123;

    x *= 23786259 - rseed;

    unsigned int t;

    t = x ^ (x << 11);
    x = y; y = z; z = w;
    return w = w ^ (w >> 19) ^ t ^ (t >> 8);
}

extern void _fini();

void exit(int status) {
    _fini();
    system_call(SC_EXIT, (unsigned long)status, 0, 0);
}

void abort() {
    system_call(SC_EXIT, EXIT_FAILURE, 0, 0);
}


int abs(int n) {
    return n < 0 ? -n : n;
}

long labs(long n) {
    return n < 0 ? -n : n;
}

long long llabs(long long n) {
    return n < 0 ? -n : n;
}

// x86_64 has faster, more reliable versions of these functions
#ifndef __x86_64__

div_t div(int numer, int denom) {
    div_t result;
    result.quot = numer / denom;
    result.rem = numer % denom;
    return result;
}

ldiv_t ldiv(long numer, long denom) {
    ldiv_t result;
    result.quot = numer / denom;
    result.rem = numer % denom;
    return result;
}

lldiv_t lldiv(long long numer, long long denom) {
    lldiv_t result;
    result.quot = numer / denom;
    result.rem = numer % denom;
    return result;
}

#endif
