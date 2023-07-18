#ifndef _KERNEL_HAL_GRAPHICS_HPP
#define _KERNEL_HAL_GRAPHICS_HPP

#include <stdint.h>

#include "Graphics.h"

#include <Memory/PageManager.hpp>

class BasicVGA {
public:
    BasicVGA();
    BasicVGA(const FrameBuffer& buffer, Position CursorPosition, uint32_t fgcolour, uint32_t bgcolour, bool double_buffer = false, WorldOS::PageManager* pm = nullptr);
    ~BasicVGA();

    void Init(const FrameBuffer& buffer, Position CursorPosition, uint32_t fgcolour, uint32_t bgcolour, bool double_buffer = false, WorldOS::PageManager* pm = nullptr);
    bool HasBeenInitialised();

    void SetFrameBuffer(const FrameBuffer& fb);
    void SetCursorPosition(Position pos);
    void SetForegroundColour(const uint32_t colour);
    void SetBackgroundColour(const uint32_t colour);

    void ClearScreen(uint32_t colour);
    void ClearScreen(uint8_t a, uint8_t r, uint8_t g, uint8_t b);
    void PlotPixel(uint64_t x, uint64_t y, uint32_t colour);
    void PlotPixel(uint64_t x, uint64_t y, uint8_t a, uint8_t r, uint8_t g, uint8_t b);

    void NewLine();
    void Backspace();
    void putc(char c);
    void ScrollText();

    Position GetCursorPosition();
    uint64_t GetScreenSizeBytes();
    FrameBuffer GetFrameBuffer();
    uint32_t GetBackgroundColour();
    uint32_t GetForegroundColour();
    uint64_t GetAmountOfTextRows();
    uint64_t GetAmountOfTextColumns();

    void EnableDoubleBuffering(WorldOS::PageManager* pm);
    void DisableDoubleBuffering();
    void SwapBuffers(bool disable_interrupts = true);
    bool isDoubleBufferEnabled();

private:
    Position m_CursorPosition;
    uint32_t m_bgcolour;
    uint32_t m_fgcolour;
    FrameBuffer m_FrameBuffer;
    bool m_HasBeenInitialised;
    WorldOS::PageManager* m_pm;
    bool m_DoubleBuffer;
    uint8_t* m_buffer;
};

#endif /* _KERNEL_HAL_GRAPHICS_HPP */