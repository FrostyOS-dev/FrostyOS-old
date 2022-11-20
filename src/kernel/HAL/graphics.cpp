#include "graphics.h"

#include <arch/x86_64/Graphics/vga-graphics.h>

void VGA_ClearScreen(const uint32_t colour) {
    WorldOS::x86_64_VGA_Graphics_ClearScreen(colour);
}

void VGA_PlotPixel(const uint64_t x, const uint64_t y, const uint32_t colour) {
    WorldOS::x86_64_VGA_Graphics_PlotPixel(x, y, colour);
}

void VGA_SetFrameBuffer(const WorldOS::FrameBuffer& fb) {
    WorldOS::x86_64_VGA_Graphics_SetFrameBuffer(fb);
}

void VGA_SetCursorPosition(const WorldOS::Position& pos) {
    WorldOS::x86_64_VGA_Graphics_SetCursorPosition(pos);
}

void VGA_SetBackgroundColour(const uint32_t colour) {
    WorldOS::x86_64_VGA_Graphics_SetBackgroundColour(colour);
}

void VGA_SetForegroundColour(const uint32_t colour) {
    WorldOS::x86_64_VGA_Graphics_SetForegroundColour(colour);
}

void VGA_NewLine() {
    WorldOS::x86_64_VGA_Graphics_NewLine();
}

void VGA_putc(const char c) {
    WorldOS::x86_64_VGA_Graphics_putc(c);
}

void VGA_puts(const char* str) {
    WorldOS::x86_64_VGA_Graphics_puts(str);
}

WorldOS::FrameBuffer VGA_GetFrameBuffer() {
    return WorldOS::x86_64_VGA_Graphics_GetFrameBuffer();
}

WorldOS::Position VGA_GetCursorPosition() {
    return WorldOS::x86_64_VGA_Graphics_GetCursorPosition();
}

uint32_t VGA_GetBackgroundColour() {
    return WorldOS::x86_64_VGA_Graphics_GetBackgroundColour();
}

uint32_t VGA_GetForegroundColour() {
    return WorldOS::x86_64_VGA_Graphics_GetForegroundColour();
}

uint64_t VGA_GetAmountOfTextRows() {
    return WorldOS::x86_64_VGA_Graphics_GetAmountOfRows();
}

uint64_t VGA_GetAmountOfTextColumns() {
    return WorldOS::x86_64_VGA_Graphics_GetAmountOfColumns();
}

uint64_t VGA_GetScreenSizeBytes() {
    return WorldOS::x86_64_VGA_Graphics_GetScreenSizeBytes();
}