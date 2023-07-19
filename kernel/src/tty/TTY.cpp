#include "TTY.hpp"

#include <util.h>

TTY* g_CurrentTTY = nullptr;

TTY::TTY() : m_VGADevice(nullptr), m_foreground(0xFF, 0xFF, 0xFF), m_background(0, 0, 0) {

}

TTY::TTY(BasicVGA* VGADevice, const Colour& fg_colour, const Colour& bg_colour) : m_VGADevice(VGADevice), m_foreground(fg_colour), m_background(bg_colour) {
    
}

TTY::~TTY() {

}


void TTY::putc(char c) {
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
            for (int i = 0; i < 4; i++) {
                m_VGADevice->putc(' ');
            }
            break;
        case '\r':
            m_VGADevice->SetCursorPosition({0, m_VGADevice->GetCursorPosition().y});
            break;
        case '\f':
            m_VGADevice->ClearScreen(m_background.as_ARGB());
            m_VGADevice->SetCursorPosition({0, 0});
            break;
        default:
            m_VGADevice->putc(c);
            break;
    }
}

void TTY::puts(const char* str) {
    if (m_VGADevice == nullptr)
        return;
    uint64_t i = 0;
    char c = str[i];
    while (c) {
        putc(c);
        c = str[++i];
    }
}


void TTY::SetDefaultForeground(const Colour& colour) {
    m_foreground = colour;
}

void TTY::SetDefaultBackground(const Colour& colour) {
    m_background = colour;
}

BasicVGA* TTY::GetVGADevice() {
    return m_VGADevice;
}

void TTY::SetVGADevice(BasicVGA* device) {
    m_VGADevice = device;
}
