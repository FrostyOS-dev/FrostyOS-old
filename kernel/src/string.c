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

#include "string.h"

size_t strlen(const char* str) {
    size_t len = 0;

    char c = str[len];

    while (c != 0) {
        len++;
        c = str[len];
    }

    return len;
}

char* strcpy(char* dst, const char* src) {
    if (dst == NULL || src == NULL)
        return dst;
    size_t i = 0;
    while (1) {
        dst[i] = src[i];
        if (src[i] == 0)
            break; // still copy the terminating character
        i++;
    }
    return dst;
}

char* strncpy(char* dst, const char* src, size_t n) {
    if (dst == NULL || src == NULL)
        return NULL;
    for (size_t i = 0; i < n; i++) {
        dst[i] = src[i];
        if (src[i] == 0)
            break; // still copy the terminating character
    }
    return dst;
}

int strcmp(const char* str1, const char* str2) {
    size_t i = 0;
    while (1) {
        if (str1[i] != str2[i]) {
            return str1[i] > str2[i] ? 1 : -1;
        }
        if (str1[i] == 0) break; // at this point we can safely assume str1[i] and str2[i] are the same so we only need to check one.
        i++;
    }
    return 0;
}

int strncmp(const char* str1, const char* str2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (str1[i] != str2[i]) {
            return str1[i] > str2[i] ? 1 : -1;
        }
    }
    return 0;
}
