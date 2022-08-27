#ifndef _KERNEL_GRAPHICS_H
#define _KERNEL_GRAPHICS_H

#include "wos-stddef.h"
#include "wos-stdint.h"

namespace WorldOS {
    struct FrameBuffer {
        void* FrameBufferAddress;
        uint64_t FrameBufferWidth;
        uint64_t FrameBufferHeight;
    };

    void PlotPixel(FrameBuffer& buffer, const uint64_t x, const uint64_t y, const uint32_t ARGB);
    void PlotPixel(FrameBuffer& buffer, const uint64_t x, const uint64_t y, const uint8_t a, const uint8_t r, const uint8_t b, const uint8_t g);

    void DrawChar(FrameBuffer& buffer, const char c, const uint64_t x, const uint64_t y, const uint32_t fgcolour, const uint32_t bgcolour);
}

#endif /* _KERNEL_GRAPHICS_H */