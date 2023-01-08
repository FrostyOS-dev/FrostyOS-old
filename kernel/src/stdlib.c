#include <stdlib.h>

#include <string.h>

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

long strtol(const char* str) {
    return atol(str);
}

unsigned long strtoul(const char* str) {
    unsigned long value = 0;
    int i = 0;
    while (str[i] != 0) {
        value += str[i] - '0';
        value *= 10;
        i++;
    }
    return value;
}
