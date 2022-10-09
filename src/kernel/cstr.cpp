#include "cstr.h"

#include "util.h"

char uintTo_StringOutput[64];
const char* to_string(uint64_t value) {
    for (uint8_t i = 0; i < 64; i++) uintTo_StringOutput[i] = 0;
    uint8_t size = 0;
    /* check length */
    size_t sizeTest = value;
    while (sizeTest / 10 > 0) {
        sizeTest /= 10;
        size++;
    }

    /* actually convert it to a string */
    uint8_t index = 0;
    while (value / 10 > 0) {
        uint8_t remainder = value % 10;
        value /= 10;
        uintTo_StringOutput[size - index] = remainder + '0';
        index++;
    }

    /* finalise the string */
    uint8_t remainder = value % 10;
    uintTo_StringOutput[size - index] = remainder + '0';
    uintTo_StringOutput[size + 1] = 0;
    return uintTo_StringOutput;
}

char intTo_StringOutput[128];
const char* to_string(int64_t value) {
    uint8_t isNegative = 0;

    if (value < 0) {
        isNegative = 1;
        value *= -1;
        intTo_StringOutput[0] = '-';
    }

    uint8_t size = 0;
    size_t sizeTest = value;
    while (sizeTest / 10 > 0) {
        sizeTest /= 10;
        size++;
    }

    uint8_t index = 0;
    while (value / 10 > 0) {
        uint8_t remainder = value % 10;
        value /= 10;
        intTo_StringOutput[isNegative + size - index] = remainder + '0';
        index++;
    }

    uint8_t remainder = value % 10;
    intTo_StringOutput[isNegative + size - index] = remainder + '0';
    intTo_StringOutput[isNegative + size + 1] = 0;
    return intTo_StringOutput;
}

/*
char doubleTo_StringOutput[128];
const char* to_string(double value, uint8_t decimalPlaces) {
    char* intPtr = (char*) to_string((int64_t)value);
    char* doublePtr = doubleTo_StringOutput;

    if (value < 0) {
        value *= -1;
    }

    while (*intPtr != 0) {
        *doublePtr = *intPtr;
        intPtr++;
        doublePtr++;
    }

    *doublePtr = '.';
    doublePtr++;

    double newValue = (int)value;

    for (uint8_t i = 0; i < decimalPlaces; i++) {
        newValue *= 10;
        *doublePtr = (int)newValue + '0';
        newValue -= (int)newValue;
        doublePtr++;
    }

    *doublePtr = 0;
    return doubleTo_StringOutput;
}
*/

char convertHDigitTo_HChar(const uint8_t value) {
    if (value > 15) return 0;

    char characters[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    return characters[value];
}

char numTo_HStringOutput[32];
const char* to_hstring(const uint64_t value) {
    memset(numTo_HStringOutput, 0, 32);

    uint64_t temp = value;
    uint64_t remainder = 0;

    char tempChars[16];
    memset(tempChars, 0, 16);


    if (value == 0) {
        numTo_HStringOutput[0] = '0';
        numTo_HStringOutput[1] = 0;
        return numTo_HStringOutput;
    }

    uint8_t index = 0;
    
    /* Convert to string (that is backwards) */
    while (true) {
        if (temp == 0) break;
        remainder = temp % 16;
        temp /= 16;

        tempChars[index] = convertHDigitTo_HChar(remainder);
        if (tempChars[index] == 0) {
            return nullptr;
        }
        index++;
    }

    const uint8_t indexTemp = index;

    /* Fix String */
    for (uint8_t i = 0; i < (indexTemp + 1); i++) {
        numTo_HStringOutput[i] = tempChars[indexTemp - i];
        index++;
    }

    /* Finalize string */

    numTo_HStringOutput[index + 1] = 0;
    
    return numTo_HStringOutput;
}

size_t strlen(const char* str) {
    size_t len = 0;

    char c = str[len];

    while (c != 0) {
        len++;
        c = str[len];
    }

    return len;
}
