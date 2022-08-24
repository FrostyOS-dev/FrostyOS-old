#include "font.h"

void getChar(uint8_t out[16], const char in) {
    for (uint8_t i = 0; i < 16; i++) { 
        out[i] = 0x00;
    }
    if (in < 32 || in > 126) return;
    out = (uint8_t*) letters[in-32];
}

