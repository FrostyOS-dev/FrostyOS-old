/*
Copyright (©) 2022-2023  Frosty515

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _X86_64_GRAPHICS_DEFINITIONS_H
#define _X86_64_GRAPHICS_DEFINITIONS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


struct Position {
    uint64_t x;
    uint64_t y;
};

struct FrameBuffer {
    void* FrameBufferAddress;
    uint64_t FrameBufferWidth;
    uint64_t FrameBufferHeight;
    uint64_t pitch;
    uint16_t bpp; // Bits per pixel
    uint8_t red_mask_size;
    uint8_t red_mask_shift;
    uint8_t green_mask_size;
    uint8_t green_mask_shift;
    uint8_t blue_mask_size;
    uint8_t blue_mask_shift;
};


#ifdef __cplusplus
}
#endif

#endif /* _X86_64_GRAPHICS_DEFINITIONS_H */