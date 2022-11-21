#include "cstring.h"

#include "util.h"

size_t strlen(const char* str) {
    size_t len = 0;

    char c = str[len];

    while (c != 0) {
        len++;
        c = str[len];
    }

    return len;
}
