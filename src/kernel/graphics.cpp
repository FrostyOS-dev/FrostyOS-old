#include "graphics.h"
#include "font.h"

namespace WorldOS {
    BasicRenderer* GlobalBasicRenderer = nullptr;

    BasicRenderer::BasicRenderer() {
        m_CursorPosition = {0,0};
        m_bgcolour = 0;
        m_fgcolour = 0xFFFFFFFF;
    }

    BasicRenderer::BasicRenderer(FrameBuffer& buffer, const Position& CursorPosition, const uint32_t fgcolour, const uint32_t bgcolour) {
        m_FrameBuffer = buffer;
        m_CursorPosition = CursorPosition;
        m_bgcolour = bgcolour;
        m_fgcolour = fgcolour;
    }

    BasicRenderer::~BasicRenderer() {

    }

    void BasicRenderer::SetFrameBuffer(FrameBuffer& buffer) {
        m_FrameBuffer = buffer;
    }

    void BasicRenderer::SetCursorPosition(Position& CursorPosition) {
        m_CursorPosition = CursorPosition;
    }

    void BasicRenderer::Print(const char* str) {
        char c = 0x00;
        uint64_t location = 0;
        while (true) {
            c = str[location++];
            if (c == 0x00) break;
            if (c == '\n') {
                NewLine();
            }
            else {
                if (m_FrameBuffer.FrameBufferWidth < (m_CursorPosition.x + 18)) {
                    NewLine();
                }
                DrawChar(c, m_CursorPosition.x, m_CursorPosition.y, m_fgcolour, m_bgcolour);
                m_CursorPosition = {m_CursorPosition.x + 10, m_CursorPosition.y};
            }
        }
    }

    void BasicRenderer::NewLine() {
        m_CursorPosition.y += 16;
        m_CursorPosition.x = 0;
    }

    void BasicRenderer::PlotPixel(const uint64_t x, const uint64_t y, const uint32_t ARGB) {
        *((uint32_t*)(m_FrameBuffer.FrameBufferAddress + 4 * m_FrameBuffer.FrameBufferWidth * y + 4 * x)) = ARGB;
    }

    void BasicRenderer::PlotPixel(const uint64_t x, const uint64_t y, const uint8_t a, const uint8_t r, const uint8_t b, const uint8_t g) {
        uint32_t ARGB;
        uint32_t A = a, R = r, G = g, B = b;
        ARGB = A << 8 | R;
        ARGB = ARGB << 8 | G;
        ARGB = ARGB << 8 | B;
        PlotPixel(x, y, ARGB);
    }

    void BasicRenderer::DrawChar(const char c, const uint64_t x, const uint64_t y, const uint32_t fgcolour, const uint32_t bgcolour) {
        uint8_t mask[16];
        GetCharReturn charReturn = getChar(c);
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
        for (uint8_t cy = 0; cy < 16; cy++) {
            int8_t inverseCX = 7;
            uint8_t temp = 0;
            bool bit = false;
            for (uint8_t cx = 0; cx < 8; cx++) {
                if (inverseCX < 0) break;
                temp = mask[cy] >> inverseCX;
                temp &= 1;
                bit = temp;
                PlotPixel(x+cx, y+cy, (bit ? fgcolour : bgcolour));
                inverseCX--;
            }
        }
    }

    void BasicRenderer::ClearScreen(const uint32_t ARGB) {
        for (uint64_t y = 0; y < m_FrameBuffer.FrameBufferHeight; y++) {
            for (uint64_t x = 0; x < m_FrameBuffer.FrameBufferWidth; x++) {
                PlotPixel(x, y, ARGB);
            }
        }
    }

    void BasicRenderer::ClearScreen(const uint8_t a, const uint8_t r, const uint8_t g, const uint8_t b) {
        for (uint64_t y = 0; y < m_FrameBuffer.FrameBufferHeight; y++) {
            for (uint64_t x = 0; x < m_FrameBuffer.FrameBufferWidth; x++) {
                PlotPixel(x, y, a, r, g, b);
            }
        }
    }

    void BasicRenderer::ScrollText(/*const bool direction*/) {

        /* Get Address of start of the second row */
        void* startAddress = (void*)((uint8_t*)m_FrameBuffer.FrameBufferAddress + 4 * m_FrameBuffer.FrameBufferWidth * 16);

        /* Copy everything up one row */
        memcpy(m_FrameBuffer.FrameBufferAddress, startAddress, (GetScreenSizeBytes() - 4 * m_FrameBuffer.FrameBufferWidth * 16));

        /* Get Address on the start of the last row */
        void* endAddress = (void*)((uint8_t*)m_FrameBuffer.FrameBufferAddress + 4 * m_FrameBuffer.FrameBufferWidth * 16 * (GetAmountOfRows() - 1));

        /* Set everything in the last row to zero */
        memset(endAddress, 0, (4 * m_FrameBuffer.FrameBufferWidth * 16));

        m_CursorPosition.y -= 1;
    }

    void BasicRenderer::SetForegroundColour(const uint32_t colour) {
        m_fgcolour = colour;
    }

    void BasicRenderer::SetBackgroundColour(const uint32_t colour) {
        m_bgcolour = colour;
    }

}