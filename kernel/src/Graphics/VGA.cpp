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

#include "VGA.hpp"
#include "VGAFont.hpp"

#include <util.h>

#ifdef __x86_64__
#include <arch/x86_64/io.h>
#endif

BasicVGA::BasicVGA() : m_CursorPosition({0, 0}), m_bgcolour(), m_fgcolour(), m_FrameBuffer({nullptr, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}), m_HasBeenInitialised(false), m_pm(nullptr), m_DoubleBuffer(false), m_buffer(nullptr) {

}

BasicVGA::BasicVGA(const FrameBuffer& buffer, Position CursorPosition, const Colour& fg_colour, const Colour& bg_colour, bool double_buffer, PageManager* pm) : m_CursorPosition(CursorPosition), m_bgcolour(bg_colour), m_fgcolour(fg_colour), m_FrameBuffer(buffer), m_HasBeenInitialised(true), m_pm(pm), m_DoubleBuffer(double_buffer), m_buffer(nullptr) {
    if (double_buffer)
        EnableDoubleBuffering(pm);
}

BasicVGA::~BasicVGA() {
    
}

void BasicVGA::Init(const FrameBuffer& buffer, Position CursorPosition, const Colour& fg_colour, const Colour& bg_colour, bool double_buffer, PageManager* pm) {
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

void BasicVGA::SetForegroundColour(const Colour& colour) {
    m_fgcolour = colour;
}

void BasicVGA::SetBackgroundColour(const Colour& colour) {
    m_bgcolour = colour;
}

void BasicVGA::ClearScreen(const Colour& colour) {
    for (uint64_t y = 0; y < m_FrameBuffer.FrameBufferHeight; y++) {
        for (uint64_t x = 0; x < m_FrameBuffer.FrameBufferWidth; x++) {
            PlotPixel(x, y, colour);
        }
    }
}

void BasicVGA::PlotPixel(uint64_t x, uint64_t y, const Colour& colour) {
    uint8_t* buffer;
    if (m_DoubleBuffer)
        buffer = m_buffer;
    else
        buffer = (uint8_t*)m_FrameBuffer.FrameBufferAddress;
    uint64_t data = colour.render();
    uint16_t bpp = m_FrameBuffer.bpp;
    if (bpp <= 8)
        buffer[m_FrameBuffer.FrameBufferWidth * y + x] = data & 0xFF;
    else if (bpp <= 16)
        *(uint16_t*)((uint64_t)buffer + m_FrameBuffer.pitch * y + 2 * x) = data & 0xFFFF;
    else if (bpp <= 24) {
        *(uint16_t*)((uint64_t)buffer + m_FrameBuffer.pitch * y + 3 * x) = data & 0xFFFF;
        buffer[m_FrameBuffer.pitch * y + 3 * x + 2] = (data & 0xFF0000) >> 16;
    }
    else if (bpp <= 32)
        *(uint32_t*)((uint64_t)buffer + m_FrameBuffer.pitch * y + 4 * x) = data & 0xFFFFFFFF;
    else if (bpp <= 40) {
        *(uint32_t*)((uint64_t)buffer + m_FrameBuffer.pitch * y + 5 * x) = data & 0xFFFFFFFF;
        buffer[m_FrameBuffer.pitch * y + 5 * x + 4] = (data & 0xFF00000000) >> 32;
    }
    else if (bpp <= 48) {
        *(uint32_t*)((uint64_t)buffer + m_FrameBuffer.pitch * y + 6 * x) = data & 0xFFFFFFFF;
        *(uint16_t*)((uint64_t)buffer + m_FrameBuffer.pitch * y + 6 * x + 4) = (data & 0xFFFF00000000) >> 32;
    }
    else if (bpp <= 56) {
        *(uint32_t*)((uint64_t)buffer + m_FrameBuffer.pitch * y + 7 * x) = data & 0xFFFFFFFF;
        *(uint16_t*)((uint64_t)buffer + m_FrameBuffer.pitch * y + 7 * x + 4) = (data & 0xFFFF00000000) >> 32;
        buffer[m_FrameBuffer.pitch * y + 7 * x + 6] = (data & 0xFF000000000000) >> 48;
    }
    else if (bpp <= 64)
        *(uint64_t*)((uint64_t)buffer + m_FrameBuffer.pitch * y + 8 * x) = data;
}

void BasicVGA::NewLine() {
    m_CursorPosition.y += 16;
    if (m_CursorPosition.y >= ALIGN_DOWN(m_FrameBuffer.FrameBufferHeight, 16))
        ScrollText();
    m_CursorPosition.x = 0;
}

void BasicVGA::Backspace() {
    if (m_CursorPosition.x < 10 && m_CursorPosition.y < 16)
        return; // Cannot backspace
    // Rewind
    if (m_CursorPosition.x < 10)
        m_CursorPosition = {ALIGN_DOWN(m_FrameBuffer.FrameBufferWidth, 10) - 10, m_CursorPosition.y - 16};
    else
        m_CursorPosition = {m_CursorPosition.x - 10, m_CursorPosition.y};
    // Zero it
    for (uint8_t cy = 0; cy < 16; cy++) {
        for (uint8_t cx = 0; cx < 10; cx++)
            PlotPixel((m_CursorPosition.x)+cx, (m_CursorPosition.y)+cy, m_bgcolour);
    }
}

void BasicVGA::putc(const char c) {

    if (c == '\n') {
        NewLine();
        return;
    }

    if (!IsCharValid(c))
        return; // don't print anything if the character is invalid
    
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
    if (m_CursorPosition.x >= ALIGN_DOWN(m_FrameBuffer.FrameBufferWidth, 10))
        NewLine();
}

void BasicVGA::ScrollText() {
    /* Copy everything up one row */
    for (uint64_t y = 16; y < (GetAmountOfTextRows() * 16); y += 16) {
        memcpy((void*)((uint64_t)(m_FrameBuffer.FrameBufferAddress) + ((y - 16) * m_FrameBuffer.pitch)), (void*)((uint64_t)m_FrameBuffer.FrameBufferAddress + (y * m_FrameBuffer.pitch)), m_FrameBuffer.pitch * 16);
    }

    /* Set everything in the last row to zero */
    memset((void*)((uint64_t)(m_FrameBuffer.FrameBufferAddress) + m_FrameBuffer.pitch * 16 * (GetAmountOfTextRows() - 1)), 0, m_FrameBuffer.pitch * 16);

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

const Colour& BasicVGA::GetBackgroundColour() {
    return m_bgcolour;
}

const Colour& BasicVGA::GetForegroundColour() {
    return m_fgcolour;
}

uint64_t BasicVGA::GetAmountOfTextRows() {
    return m_FrameBuffer.FrameBufferHeight / 16;
}

uint64_t BasicVGA::GetAmountOfTextColumns() {
    return m_FrameBuffer.FrameBufferWidth / 10;
}

void BasicVGA::EnableDoubleBuffering(PageManager* pm) {
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
