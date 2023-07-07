#include "VGA.hpp"
#include "VGAFont.hpp"

#include <util.h>

static Position g_CursorPosition = {0,0};
static uint32_t g_bgcolour = 0;
static uint32_t g_fgcolour = 0;
static FrameBuffer g_FrameBuffer = {nullptr, 0, 0, 0};
static bool g_HasBeenInitialised = false;

void VGA_Init(const FrameBuffer& buffer, Position CursorPosition, uint32_t fgcolour, uint32_t bgcolour) {
    g_FrameBuffer = buffer;
    g_CursorPosition = CursorPosition;
    g_fgcolour = fgcolour;
    g_bgcolour = bgcolour;
}

bool VGA_HasBeenInitialised() {
    return g_HasBeenInitialised;
}

void VGA_SetFrameBuffer(const FrameBuffer& buffer) {
    g_FrameBuffer = buffer;
}

void VGA_SetCursorPosition(Position CursorPosition) {
    g_CursorPosition = CursorPosition;
}

void VGA_SetForegroundColour(const uint32_t colour) {
    g_fgcolour = colour;
}

void VGA_SetBackgroundColour(const uint32_t colour) {
    g_bgcolour = colour;
}

void VGA_ClearScreen(uint32_t ARGB) {
    for (uint64_t y = 0; y < g_FrameBuffer.FrameBufferHeight; y++) {
        for (uint64_t x = 0; x < g_FrameBuffer.FrameBufferWidth; x++) {
            VGA_PlotPixel(x, y, ARGB);
        }
    }
}

void VGA_ClearScreen(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    for (uint64_t y = 0; y < g_FrameBuffer.FrameBufferHeight; y++) {
        for (uint64_t x = 0; x < g_FrameBuffer.FrameBufferWidth; x++) {
            VGA_PlotPixel(x, y, a, r, g, b);
        }
    }
}

void VGA_PlotPixel(uint64_t x, uint64_t y, uint32_t ARGB) {
    *((uint32_t*)(g_FrameBuffer.FrameBufferAddress + 4 * g_FrameBuffer.FrameBufferWidth * y + 4 * x)) = ARGB;
}

void VGA_PlotPixel(uint64_t x, uint64_t y, uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    uint32_t ARGB = (a << 8) | r;
    ARGB = (ARGB << 8) | g;
    ARGB = (ARGB << 8) | b;
    VGA_PlotPixel(x, y, ARGB);
}

void VGA_NewLine() {
    g_CursorPosition.y += 16;
    if (g_CursorPosition.y >= g_FrameBuffer.FrameBufferHeight)
        VGA_ScrollText();
    g_CursorPosition.x = 0;
}

void VGA_puts(const char* str) {
    char c = 0x00;
    uint64_t location = 0;
    while (true) {
        c = str[location++];
        if (c == 0x00) break;
        if (c == '\n') {
            VGA_NewLine();
        }
        else {
            if (g_FrameBuffer.FrameBufferWidth < (g_CursorPosition.x + 18)) {
                VGA_NewLine();
            }
            VGA_putc(c);
        }
    }
}

void VGA_putc(const char c) {

    if (c == '\n') {
        VGA_NewLine();
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
            VGA_PlotPixel((g_CursorPosition.x)+cx, (g_CursorPosition.y)+cy, (bit ? g_fgcolour : g_bgcolour));
            inverseCX--;
        }
    }
    /* Adjust Cursor position to say a character has been printed */
    g_CursorPosition = {g_CursorPosition.x + 10, g_CursorPosition.y};
}

void VGA_ScrollText() {
    /* Copy everything up one row */
    for (uint64_t y = 16; y < ((VGA_GetAmountOfTextRows() - 1) * 16); y += 16)
        fast_memmove((g_FrameBuffer.FrameBufferAddress + ((y - 16) * 4 * g_FrameBuffer.FrameBufferWidth)), (g_FrameBuffer.FrameBufferAddress + (y * 4 * g_FrameBuffer.FrameBufferWidth)), 4 * g_FrameBuffer.FrameBufferWidth);

    /* Set everything in the last row to zero */
    fast_memset((void*)((uint64_t)(g_FrameBuffer.FrameBufferAddress) + 4 * g_FrameBuffer.FrameBufferWidth * 16 * (VGA_GetAmountOfTextRows() - 1)), 0, (4 * g_FrameBuffer.FrameBufferWidth * 16) / 8);

    g_CursorPosition.y -= 16;
}

Position VGA_GetCursorPosition() {
    return g_CursorPosition;
}

uint64_t VGA_GetScreenSizeBytes() {
    return g_FrameBuffer.FrameBufferWidth * g_FrameBuffer.FrameBufferHeight * 4;
}

FrameBuffer VGA_GetFrameBuffer() {
    return g_FrameBuffer;
}

uint32_t VGA_GetBackgroundColour() {
    return g_bgcolour;
}

uint32_t VGA_GetForegroundColour() {
    return g_fgcolour;
}

uint64_t VGA_GetAmountOfTextRows() {
    return g_FrameBuffer.FrameBufferHeight / 16;
}

uint64_t VGA_GetAmountOfTextColumns() {
    return g_FrameBuffer.FrameBufferWidth / 10;
}
