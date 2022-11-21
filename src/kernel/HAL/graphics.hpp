#ifndef _KERNEL_HAL_GRAPHICS_HPP
#define _KERNEL_HAL_GRAPHICS_HPP

#include <stdint.h>

#include <arch/x86_64/Graphics/graphics-defs.h>

void VGA_ClearScreen(const uint32_t colour);
void VGA_PlotPixel(const uint64_t x, const uint64_t y, const uint32_t colour);
void VGA_SetFrameBuffer(const FrameBuffer fb);
void VGA_SetCursorPosition(const Position pos);
void VGA_SetBackgroundColour(const uint32_t colour);
void VGA_SetForegroundColour(const uint32_t colour);
void VGA_NewLine();

void VGA_putc(const char c);
void VGA_puts(const char* str);

FrameBuffer VGA_GetFrameBuffer();
Position VGA_GetCursorPosition();
uint32_t VGA_GetBackgroundColour();
uint32_t VGA_GetForegroundColour();
uint64_t VGA_GetAmountOfTextRows();
uint64_t VGA_GetAmountOfTextColumns();
uint64_t VGA_GetScreenSizeBytes();

#endif /* _KERNEL_HAL_GRAPHICS_HPP */