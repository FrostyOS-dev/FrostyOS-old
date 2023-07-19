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
    uint16_t bpp; // Bits per pixel
};


#ifdef __cplusplus
}
#endif

#endif /* _X86_64_GRAPHICS_DEFINITIONS_H */