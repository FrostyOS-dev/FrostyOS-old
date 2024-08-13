#include "VGATTY.hpp"

#include <math.h>

#include <Graphics/Colour.hpp>
#include <Graphics/VGA.hpp>

TTYBackendVGA::TTYBackendVGA() : TTYBackend(TTYBackendType::VGA, TTYBackendStreamDirection::OUTPUT), m_VGADevice(nullptr), m_fg_colour(), m_bg_colour() {

}

TTYBackendVGA::TTYBackendVGA(BasicVGA* VGADevice, const Colour& fg_colour, const Colour& bg_colour) : TTYBackend(TTYBackendType::VGA, TTYBackendStreamDirection::OUTPUT), m_VGADevice(VGADevice), m_fg_colour(fg_colour), m_bg_colour(bg_colour) {

}

TTYBackendVGA::~TTYBackendVGA() {

}

void TTYBackendVGA::putc(char c) {
    if (m_VGADevice == nullptr)
        return;
    switch (c) {
        case '\b':
            m_VGADevice->Backspace();
            break;
        case '\a':
            break; // Do nothing
        case '\n':
        case '\v':
            m_VGADevice->NewLine();
            break;
        case '\t':
            for (int i = 0; i < 4; i++)
                m_VGADevice->putc(' ');
            break;
        case '\r':
            m_VGADevice->SetCursorPosition({0, m_VGADevice->GetCursorPosition().y});
            break;
        case '\f':
            m_VGADevice->ClearScreen(m_bg_colour);
            m_VGADevice->SetCursorPosition({0, 0});
            break;
        default:
            m_VGADevice->putc(c);
            break;
    }
}

void TTYBackendVGA::puts(const char* str) {
    if (m_VGADevice == nullptr)
        return;
    for (size_t i = 0; str[i] != '\0'; i++)
        putc(str[i]);
}

void TTYBackendVGA::SetDefaultForeground(const Colour& colour) {
    m_fg_colour = colour;
}

void TTYBackendVGA::SetDefaultBackground(const Colour& colour) {
    m_bg_colour = colour;
}

BasicVGA* TTYBackendVGA::GetVGADevice() {
    return m_VGADevice;
}

void TTYBackendVGA::SetVGADevice(BasicVGA* device) {
    m_VGADevice = device;
}

void TTYBackendVGA::SwapBuffers() {
    if (m_VGADevice == nullptr)
        return;
    m_VGADevice->SwapBuffers();
}

void TTYBackendVGA::seek(size_t offset) {
    if (m_VGADevice == nullptr)
        return;
    uldiv_t out = uldiv(offset, m_VGADevice->GetAmountOfTextRows());
    m_VGADevice->SetCursorPosition({out.rem * 10, out.quot * 16});
}

size_t TTYBackendVGA::tell() {
    if (m_VGADevice == nullptr)
        return 0;
    size_t x_col = m_VGADevice->GetCursorPosition().x / 10;
    size_t y_row = m_VGADevice->GetCursorPosition().y / 16;
    return y_row * m_VGADevice->GetAmountOfTextRows() + x_col;
}
