#ifndef _KERNEL_X86_64_VGA_GRAPHICS_H
#define _KERNEL_X86_64_VGA_GRAPHICS_H

#include <stddef.h>
#include <stdint.h>

#include <util.h>

#include "graphics-defs.h"

namespace WorldOS {

    void x86_64_VGA_Graphics_Init(const FrameBuffer& buffer, const Position& CursorPosition, const uint32_t fgcolour, const uint32_t bgcolour);
    bool x86_64_VGA_Graphics_HasBeenInitialised();
    void x86_64_VGA_Graphics_SetFrameBuffer(const FrameBuffer& buffer);
    void x86_64_VGA_Graphics_SetCursorPosition(const Position& CursorPosition);
    void x86_64_VGA_Graphics_puts(const char* str);
    void x86_64_VGA_Graphics_NewLine();
    void x86_64_VGA_Graphics_PlotPixel(const uint64_t x, const uint64_t y, const uint32_t ARGB);
    void x86_64_VGA_Graphics_PlotPixel(const uint64_t x, const uint64_t y, const uint8_t a, const uint8_t r, const uint8_t b, const uint8_t g);
    void x86_64_VGA_Graphics_putc(const char c);
    void x86_64_VGA_Graphics_ClearScreen(const uint32_t ARGB);
    void x86_64_VGA_Graphics_ClearScreen(const uint8_t a, const uint8_t r, const uint8_t g, const uint8_t b);

    Position x86_64_VGA_Graphics_GetCursorPosition();
    uint64_t x86_64_VGA_Graphics_GetScreenSizeBytes();
    FrameBuffer x86_64_VGA_Graphics_GetFrameBuffer();
    uint32_t x86_64_VGA_Graphics_GetBackgroundColour();
    uint32_t x86_64_VGA_Graphics_GetForegroundColour();
    uint64_t x86_64_VGA_Graphics_GetAmountOfRows();
    uint64_t x86_64_VGA_Graphics_GetAmountOfColumns();
    
    void x86_64_VGA_Graphics_ScrollText(/*const bool direction*/); // directional scrolling not implemented yet, only scroll up (move text up)
    void x86_64_VGA_Graphics_SetForegroundColour(const uint32_t colour);
    void x86_64_VGA_Graphics_SetBackgroundColour(const uint32_t colour);
}

#endif /* _KERNEL_X86_64_VGA_GRAPHICS_H */