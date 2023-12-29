#include <inttypes.h>

#include <stddef.h>

intmax_t imaxabs(intmax_t j) {
    return j < 0 ? -j : j;
}

// x86_64 has an architecture specific optimised and more reliable implementation of imaxdiv.
#ifndef __x86_64__

imaxdiv_t imaxdiv(intmax_t numer, intmax_t denom) {
    imaxdiv_t result;
    result.quot = numer / denom;
    result.rem = numer % denom;
    return result;
}

#endif

intmax_t strtoimax(const char* str, char** endptr, int base) {
    intmax_t value = 0;
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

uintmax_t strtoumax(const char* str, char** endptr, int base) {
    uintmax_t value = 0;
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
