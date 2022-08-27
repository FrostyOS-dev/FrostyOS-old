#include "graphics.h"
#include "font.h"

namespace WorldOS {
    void PlotPixel(FrameBuffer& buffer, const uint64_t x, const uint64_t y, const uint32_t ARGB) {
        *((uint32_t*)(buffer.FrameBufferAddress + 4 * buffer.FrameBufferWidth * y + 4 * x)) = ARGB;
    }

    void PlotPixel(FrameBuffer& buffer, const uint64_t x, const uint64_t y, const uint8_t a, const uint8_t r, const uint8_t b, const uint8_t g) {
        uint32_t ARGB;
        uint32_t A = a, R = r, G = g, B = b;
        ARGB = A << 8 | R;
        ARGB = ARGB << 8 | G;
        ARGB = ARGB << 8 | B;
        PlotPixel(buffer, x, y, ARGB);
    }

    void DrawChar(FrameBuffer& buffer, const char c, const uint64_t x, const uint64_t y, const uint32_t fgcolour, const uint32_t bgcolour) {
        uint8_t mask[16];
        GetCharReturn charReturn = getChar(c);
        mask[0] = charReturn.b0;
        mask[1] = charReturn.b1;
        mask[2] = charReturn.b2;
        mask[3] = charReturn.b3;
        mask[4] = charReturn.b4;
        mask[5] = charReturn.b5;
        mask[6] = charReturn.b6;
        mask[7] = charReturn.b7;
        mask[8] = charReturn.b8;
        mask[9] = charReturn.b9;
        mask[10] = charReturn.b10;
        mask[11] = charReturn.b11;
        mask[12] = charReturn.b12;
        mask[13] = charReturn.b13;
        mask[14] = charReturn.b14;
        mask[15] = charReturn.b15;
        for (uint8_t cy = 0; cy < 16; cy++) {
            int8_t inverseCX = 7;
            uint8_t temp = 0;
            bool bit = false;
            for (uint8_t cx = 0; cx < 8; cx++) {
                if (inverseCX < 0) break;
                temp = mask[cy] >> inverseCX;
                temp &= 1;
                bit = temp;
                PlotPixel(buffer, x+cx, y+cy, (bit ? fgcolour : bgcolour));
                inverseCX--;
            }
        }
    }
}