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
