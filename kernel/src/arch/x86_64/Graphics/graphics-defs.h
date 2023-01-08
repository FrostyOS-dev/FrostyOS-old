#ifndef _X86_64_GRAPHICS_DEFINITIONS_H
#define _X86_64_GRAPHICS_DEFINITIONS_H

#include <stdint.h>

typedef struct {
    uint64_t x;
    uint64_t y;
} Position;

typedef struct {
    void* FrameBufferAddress;
    uint64_t FrameBufferWidth;
    uint64_t FrameBufferHeight;
    uint16_t bpp; // Bits per pixel
} FrameBuffer;


#endif /* _X86_64_GRAPHICS_DEFINITIONS_H */