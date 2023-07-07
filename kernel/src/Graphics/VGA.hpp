#ifndef _KERNEL_HAL_GRAPHICS_HPP
#define _KERNEL_HAL_GRAPHICS_HPP

#include <stdint.h>

#include "Graphics.h"

void VGA_Init(const FrameBuffer& buffer, Position CusorPosition, uint32_t fgcolour, uint32_t bgcolour);
bool VGA_HasBeenInitialised();
void VGA_SetFrameBuffer(const FrameBuffer& fb);
void VGA_SetCursorPosition(Position pos);
void VGA_SetForegroundColour(const uint32_t colour);
void VGA_SetBackgroundColour(const uint32_t colour);
void VGA_ClearScreen(uint32_t colour);
void VGA_ClearScreen(uint8_t a, uint8_t r, uint8_t g, uint8_t b);
void VGA_PlotPixel(uint64_t x, uint64_t y, uint32_t colour);
void VGA_PlotPixel(uint64_t x, uint64_t y, uint8_t a, uint8_t r, uint8_t g, uint8_t b);

void VGA_NewLine();
void VGA_puts(const char* str);
void VGA_putc(const char c);
void VGA_ScrollText();

Position VGA_GetCursorPosition();
uint64_t VGA_GetScreenSizeBytes();
FrameBuffer VGA_GetFrameBuffer();
uint32_t VGA_GetBackgroundColour();
uint32_t VGA_GetForegroundColour();
uint64_t VGA_GetAmountOfTextRows();
uint64_t VGA_GetAmountOfTextColumns();

#endif /* _KERNEL_HAL_GRAPHICS_HPP */