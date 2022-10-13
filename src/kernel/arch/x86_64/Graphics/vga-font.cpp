#include "vga-font.h"

GetCharReturn getChar(const char in) {
    uint8_t out[16];
    for (uint8_t i = 0; i < 16; i++) { 
        out[i] = 0x00;
    }
    if (in < 32 || in > 126) return {out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7], out[8], out[9], out[10], out[11], out[12], out[13], out[14], out[15]};
    for (uint8_t i = 0; i < 16; i++) { 
        out[i] = letters[in-32][i];
    }
    return {out[15], out[14], out[13], out[12], out[11], out[10], out[9], out[8], out[7], out[6], out[5], out[4], out[3], out[2], out[1], out[0]};
}

