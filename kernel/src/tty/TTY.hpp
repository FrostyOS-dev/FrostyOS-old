#ifndef _KERNEL_TTY_HPP
#define _KERNEL_TTY_HPP

#include <Graphics/Colour.hpp>
#include <Graphics/VGA.hpp>

#include <Memory/PageManager.hpp>

class TTY {
public:
    TTY();
    TTY(BasicVGA* VGADevice, const Colour& fg_colour, const Colour& bg_colour); // Set the default foreground and background colours.
    ~TTY();

    void putc(char c);
    void puts(const char* str);

    void SetDefaultForeground(const Colour& colour);
    void SetDefaultBackground(const Colour& colour);

    BasicVGA* GetVGADevice();
    void SetVGADevice(BasicVGA* device);

private:
    BasicVGA* m_VGADevice;
    Colour m_foreground;
    Colour m_background;
};

extern TTY* g_CurrentTTY;

#endif /* _KERNEL_TTY_HPP */