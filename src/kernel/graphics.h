#ifndef _KERNEL_GRAPHICS_H
#define _KERNEL_GRAPHICS_H

#include "wos-stddef.h"
#include "wos-stdint.h"

#include "util.h"

namespace WorldOS {

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

    class BasicRenderer {
    public:
        BasicRenderer();
        BasicRenderer(FrameBuffer& buffer, const Position& CursorPosition = {0,0}, const uint32_t fgcolour = 0xFFFFFFFF, const uint32_t bgcolour = 0);
        ~BasicRenderer();

        void SetFrameBuffer(FrameBuffer& buffer);
        void SetCursorPosition(Position& CursorPosition);

        void Print(const char* str);
        void NewLine();

        void PlotPixel(const uint64_t x, const uint64_t y, const uint32_t ARGB);
        void PlotPixel(const uint64_t x, const uint64_t y, const uint8_t a, const uint8_t r, const uint8_t b, const uint8_t g);

        void DrawChar(const char c, const uint64_t x, const uint64_t y, const uint32_t fgcolour, const uint32_t bgcolour);

        void ClearScreen(const uint32_t ARGB);
        void ClearScreen(const uint8_t a, const uint8_t r, const uint8_t g, const uint8_t b);

        void ScrollText(/*const bool direction*/); // true for move text UP and false for move text DOWN (no directional scrolling yet, just moving the text UP)

        void SetForegroundColour(const uint32_t colour);
        void SetBackgroundColour(const uint32_t colour);

        inline Position GetCursorPosition() { return m_CursorPosition; };
        inline uint64_t GetScreenSizeBytes() { return m_FrameBuffer.FrameBufferWidth * m_FrameBuffer.FrameBufferHeight * 4; };
        inline FrameBuffer GetFrameBuffer() { return m_FrameBuffer; };
        inline uint64_t GetAmountOfRows() { return m_FrameBuffer.FrameBufferHeight / 16; };
        inline uint64_t GetAmountOfColumns() { return m_FrameBuffer.FrameBufferWidth / 10; };

    private:
        Position m_CursorPosition;
        FrameBuffer m_FrameBuffer;
        uint32_t m_bgcolour;
        uint32_t m_fgcolour;
    };

    extern BasicRenderer* GlobalBasicRenderer;
}

#endif /* _KERNEL_GRAPHICS_H */