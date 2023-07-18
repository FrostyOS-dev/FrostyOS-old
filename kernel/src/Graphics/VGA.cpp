#include "VGA.hpp"
#include "VGAFont.hpp"

#include <util.h>

#ifdef __x86_64__
#include <arch/x86_64/io.h>
#endif

BasicVGA::BasicVGA() : m_CursorPosition({0, 0}), m_bgcolour(0), m_fgcolour(0xFFFFFFFF), m_FrameBuffer({nullptr, 0, 0, 0}), m_HasBeenInitialised(false), m_pm(nullptr), m_DoubleBuffer(false), m_buffer(nullptr) {

}

BasicVGA::BasicVGA(const FrameBuffer& buffer, Position CursorPosition, uint32_t fg_colour, uint32_t bg_colour, bool double_buffer, WorldOS::PageManager* pm) : m_CursorPosition(CursorPosition), m_bgcolour(bg_colour), m_fgcolour(fg_colour), m_FrameBuffer(buffer), m_HasBeenInitialised(true), m_pm(pm), m_DoubleBuffer(double_buffer), m_buffer(nullptr) {
    if (double_buffer)
        EnableDoubleBuffering(pm);
}

BasicVGA::~BasicVGA() {
    
}

void BasicVGA::Init(const FrameBuffer& buffer, Position CursorPosition, uint32_t fg_colour, uint32_t bg_colour, bool double_buffer, WorldOS::PageManager* pm) {
    m_FrameBuffer = buffer;
    m_CursorPosition = CursorPosition;
    m_fgcolour = fg_colour;
    m_bgcolour = bg_colour;
    m_HasBeenInitialised = true;
    m_pm = pm;
    m_DoubleBuffer = double_buffer;
    m_buffer = nullptr;
    if (double_buffer)
        EnableDoubleBuffering(pm);
}

bool BasicVGA::HasBeenInitialised() {
    return m_HasBeenInitialised;
}

void BasicVGA::SetFrameBuffer(const FrameBuffer& buffer) {
    m_FrameBuffer = buffer;
}

void BasicVGA::SetCursorPosition(Position CursorPosition) {
    m_CursorPosition = CursorPosition;
}

void BasicVGA::SetForegroundColour(const uint32_t colour) {
    m_fgcolour = colour;
}

void BasicVGA::SetBackgroundColour(const uint32_t colour) {
    m_bgcolour = colour;
}

void BasicVGA::ClearScreen(uint32_t ARGB) {
    for (uint64_t y = 0; y < m_FrameBuffer.FrameBufferHeight; y++) {
        for (uint64_t x = 0; x < m_FrameBuffer.FrameBufferWidth; x++) {
            PlotPixel(x, y, ARGB);
        }
    }
}

void BasicVGA::ClearScreen(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    for (uint64_t y = 0; y < m_FrameBuffer.FrameBufferHeight; y++) {
        for (uint64_t x = 0; x < m_FrameBuffer.FrameBufferWidth; x++) {
            PlotPixel(x, y, a, r, g, b);
        }
    }
}

void BasicVGA::PlotPixel(uint64_t x, uint64_t y, uint32_t ARGB) {
    if (m_DoubleBuffer)
        *((uint32_t*)(m_buffer + 4 * m_FrameBuffer.FrameBufferWidth * y + 4 * x)) = ARGB;
    else
        *((uint32_t*)(m_FrameBuffer.FrameBufferAddress + 4 * m_FrameBuffer.FrameBufferWidth * y + 4 * x)) = ARGB;
}

void BasicVGA::PlotPixel(uint64_t x, uint64_t y, uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    uint32_t ARGB = (a << 8) | r;
    ARGB = (ARGB << 8) | g;
    ARGB = (ARGB << 8) | b;
    PlotPixel(x, y, ARGB);
}

void BasicVGA::NewLine() {
    m_CursorPosition.y += 16;
    if (m_CursorPosition.y >= m_FrameBuffer.FrameBufferHeight)
        ScrollText();
    m_CursorPosition.x = 0;
}

void BasicVGA::Backspace() {
    if (m_CursorPosition.x < 10 && m_CursorPosition.y < 10)
        return; // Cannot backspace
    // Rewind
    if (m_CursorPosition.x < 10)
        m_CursorPosition = {GetAmountOfTextColumns() - 1, m_CursorPosition.y - 16};
    else
        m_CursorPosition = {m_CursorPosition.x - 10, m_CursorPosition.y};
    putc(' '); // clear the existing character
    // Rewind again
    if (m_CursorPosition.x < 10)
        m_CursorPosition = {GetAmountOfTextColumns() - 1, m_CursorPosition.y - 16};
    else
        m_CursorPosition = {m_CursorPosition.x - 10, m_CursorPosition.y};
}

void BasicVGA::putc(const char c) {

    if (c == '\n') {
        NewLine();
        return;
    }

    /* Prepare */
    uint8_t mask[16];
    GetCharReturn charReturn = getChar(c);

    /* Following looks weird, but is necessary */
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

    /* Go through each pixel in the character and draw the foreground colour if bit is 1, else draw background colour */
    for (uint8_t cy = 0; cy < 16; cy++) {
        int8_t inverseCX = 7;
        uint8_t temp = 0;
        bool bit = false;
        for (uint8_t cx = 0; cx < 8; cx++) {
            if (inverseCX < 0) break;
            temp = mask[cy] >> inverseCX;
            temp &= 1;
            bit = temp;
            PlotPixel((m_CursorPosition.x)+cx, (m_CursorPosition.y)+cy, (bit ? m_fgcolour : m_bgcolour));
            inverseCX--;
        }
    }
    /* Adjust Cursor position to say a character has been printed */
    m_CursorPosition = {m_CursorPosition.x + 10, m_CursorPosition.y};
}

void BasicVGA::ScrollText() {
    /* Copy everything up one row */
    for (uint64_t y = 16; y < ((GetAmountOfTextRows() - 1) * 16); y += 16)
        fast_memmove((m_FrameBuffer.FrameBufferAddress + ((y - 16) * 4 * m_FrameBuffer.FrameBufferWidth)), (m_FrameBuffer.FrameBufferAddress + (y * 4 * m_FrameBuffer.FrameBufferWidth)), 4 * m_FrameBuffer.FrameBufferWidth);

    /* Set everything in the last row to zero */
    fast_memset((void*)((uint64_t)(m_FrameBuffer.FrameBufferAddress) + 4 * m_FrameBuffer.FrameBufferWidth * 16 * (GetAmountOfTextRows() - 1)), 0, (4 * m_FrameBuffer.FrameBufferWidth * 16) / 8);

    m_CursorPosition.y -= 16;
}

Position BasicVGA::GetCursorPosition() {
    return m_CursorPosition;
}

uint64_t BasicVGA::GetScreenSizeBytes() {
    return m_FrameBuffer.FrameBufferWidth * m_FrameBuffer.FrameBufferHeight * 4;
}

FrameBuffer BasicVGA::GetFrameBuffer() {
    return m_FrameBuffer;
}

uint32_t BasicVGA::GetBackgroundColour() {
    return m_bgcolour;
}

uint32_t BasicVGA::GetForegroundColour() {
    return m_fgcolour;
}

uint64_t BasicVGA::GetAmountOfTextRows() {
    return m_FrameBuffer.FrameBufferHeight / 16;
}

uint64_t BasicVGA::GetAmountOfTextColumns() {
    return m_FrameBuffer.FrameBufferWidth / 10;
}

void BasicVGA::EnableDoubleBuffering(WorldOS::PageManager* pm) {
    if (pm == nullptr)
        return;
    m_pm = pm;
    m_buffer = (uint8_t*)m_pm->AllocatePages(DIV_ROUNDUP(GetScreenSizeBytes(), 0x1000));
    if (m_buffer == nullptr)
        return;
    fast_memset(m_buffer, 0, ALIGN_UP(GetScreenSizeBytes(), 0x1000) / 8);
    m_DoubleBuffer = true;
}

void BasicVGA::DisableDoubleBuffering() {
    if (m_pm == nullptr)
        return;
    m_DoubleBuffer = false;
    m_pm->FreePages(m_buffer);
}

void BasicVGA::SwapBuffers(bool disable_interrupts) {
    if (!m_DoubleBuffer)
        return;
    if (disable_interrupts) {
#ifdef __x86_64__
        x86_64_DisableInterrupts();
#endif
    }
    fast_memcpy(m_FrameBuffer.FrameBufferAddress, m_buffer, GetScreenSizeBytes());
    if (disable_interrupts) {
#ifdef __x86_64__
        x86_64_EnableInterrupts();
#endif
    }
}

bool BasicVGA::isDoubleBufferEnabled() {
    return m_DoubleBuffer;
}
