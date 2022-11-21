#include "vga-graphics.hpp"
#include "vga-font.hpp"

namespace WorldOS {

    static Position g_CursorPosition = {0,0};
    static uint32_t g_bgcolour = 0;
    static uint32_t g_fgcolour = 0;
    static FrameBuffer g_FrameBuffer = {nullptr, 0, 0, 0};
    static bool g_HasBeenInitialised = false;

    void x86_64_VGA_Graphics_Init(const FrameBuffer& buffer, const Position& CursorPosition, const uint32_t fgcolour, const uint32_t bgcolour) {
        g_FrameBuffer = buffer;
        g_CursorPosition = CursorPosition;
        g_fgcolour = fgcolour;
        g_bgcolour = bgcolour;
    }

    bool x86_64_VGA_Graphics_HasBeenInitialised() {
        return g_HasBeenInitialised;
    }

    void x86_64_VGA_Graphics_SetFrameBuffer(const FrameBuffer& buffer) {
        g_FrameBuffer = buffer;
    }

    void x86_64_VGA_Graphics_SetCursorPosition(const Position& CursorPosition) {
        g_CursorPosition = CursorPosition;
    }

    void x86_64_VGA_Graphics_puts(const char* str) {
        char c = 0x00;
        uint64_t location = 0;
        while (true) {
            c = str[location++];
            if (c == 0x00) break;
            if (c == '\n') {
                x86_64_VGA_Graphics_NewLine();
            }
            else {
                if (g_FrameBuffer.FrameBufferWidth < (g_CursorPosition.x + 18)) {
                    x86_64_VGA_Graphics_NewLine();
                }
                x86_64_VGA_Graphics_putc(c);
            }
        }
    }

    void x86_64_VGA_Graphics_NewLine() {
        g_CursorPosition.y += 16;
        g_CursorPosition.x = 0;
    }

    void x86_64_VGA_Graphics_PlotPixel(const uint64_t x, const uint64_t y, const uint32_t ARGB) {
        *((uint32_t*)(g_FrameBuffer.FrameBufferAddress + 4 * g_FrameBuffer.FrameBufferWidth * y + 4 * x)) = ARGB;
    }

    void x86_64_VGA_Graphics_PlotPixel(const uint64_t x, const uint64_t y, const uint8_t a, const uint8_t r, const uint8_t b, const uint8_t g) {
        uint32_t ARGB;
        uint32_t A = a, R = r, G = g, B = b;
        ARGB = A << 8 | R;
        ARGB = ARGB << 8 | G;
        ARGB = ARGB << 8 | B;
        x86_64_VGA_Graphics_PlotPixel(x, y, ARGB);
    }

    void x86_64_VGA_Graphics_putc(const char c) {

        if (c == '\n') {
            x86_64_VGA_Graphics_NewLine();
            return;
        }

        /* Prepare */
        uint8_t mask[16];
        GetCharReturn charReturn = getChar(c);

        /* Following looks weird, but is necessary */
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

        /* Go through each pixel in the character and draw the foreground colour if bit is 1, else draw background colour */
        for (uint8_t cy = 0; cy < 16; cy++) {
            int8_t inverseCX = 7;
            uint8_t temp = 0;
            bool bit = false;
            for (uint8_t cx = 0; cx < 8; cx++) {
                if (inverseCX < 0) break;
                temp = mask[cy] >> inverseCX;
                temp &= 1;
                bit = temp;
                x86_64_VGA_Graphics_PlotPixel((g_CursorPosition.x)+cx, (g_CursorPosition.y)+cy, (bit ? g_fgcolour : g_bgcolour));
                inverseCX--;
            }
        }
        /* Adjust Cursor position to say a character has been printed */
        g_CursorPosition = {g_CursorPosition.x + 10, g_CursorPosition.y};
    }

    void x86_64_VGA_Graphics_ClearScreen(const uint32_t ARGB) {
        for (uint64_t y = 0; y < g_FrameBuffer.FrameBufferHeight; y++) {
            for (uint64_t x = 0; x < g_FrameBuffer.FrameBufferWidth; x++) {
                x86_64_VGA_Graphics_PlotPixel(x, y, ARGB);
            }
        }
    }

    void x86_64_VGA_Graphics_ClearScreen(const uint8_t a, const uint8_t r, const uint8_t g, const uint8_t b) {
        for (uint64_t y = 0; y < g_FrameBuffer.FrameBufferHeight; y++) {
            for (uint64_t x = 0; x < g_FrameBuffer.FrameBufferWidth; x++) {
                x86_64_VGA_Graphics_PlotPixel(x, y, a, r, g, b);
            }
        }
    }

    Position x86_64_VGA_Graphics_GetCursorPosition() {
        return g_CursorPosition;
    }

    uint64_t x86_64_VGA_Graphics_GetScreenSizeBytes() {
        return g_FrameBuffer.FrameBufferWidth * g_FrameBuffer.FrameBufferHeight * 4;
    }

    FrameBuffer x86_64_VGA_Graphics_GetFrameBuffer() {
        return g_FrameBuffer;
    }

    uint32_t x86_64_VGA_Graphics_GetBackgroundColour() {
        return g_bgcolour;
    }

    uint32_t x86_64_VGA_Graphics_GetForegroundColour() {
        return g_fgcolour;
    }

    uint64_t x86_64_VGA_Graphics_GetAmountOfRows() {
        return g_FrameBuffer.FrameBufferHeight / 16;
    }

    uint64_t x86_64_VGA_Graphics_GetAmountOfColumns() {
        return g_FrameBuffer.FrameBufferWidth / 10;
    }

    void x86_64_VGA_Graphics_ScrollText(/*const bool direction*/) {

        /* Get Address of start of the second row */
        void* startAddress = (void*)((uint8_t*)g_FrameBuffer.FrameBufferAddress + 4 * g_FrameBuffer.FrameBufferWidth * 16);

        /* Copy everything up one row */
        memcpy(g_FrameBuffer.FrameBufferAddress, startAddress, (x86_64_VGA_Graphics_GetScreenSizeBytes() - 4 * g_FrameBuffer.FrameBufferWidth * 16)); // potential ERROR point

        /* Get Address on the start of the last row */
        void* endAddress = (void*)((uint8_t*)g_FrameBuffer.FrameBufferAddress + 4 * g_FrameBuffer.FrameBufferWidth * 16 * (x86_64_VGA_Graphics_GetAmountOfRows() - 1));

        /* Set everything in the last row to zero */
        memset(endAddress, 0, (4 * g_FrameBuffer.FrameBufferWidth * 16));

        g_CursorPosition.y -= 1;
    }

    void x86_64_VGA_Graphics_SetForegroundColour(const uint32_t colour) {
        g_fgcolour = colour;
    }

    void x86_64_VGA_Graphics_SetBackgroundColour(const uint32_t colour) {
        g_bgcolour = colour;
    }

}