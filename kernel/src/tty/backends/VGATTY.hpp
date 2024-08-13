#ifndef _TTY_BACKEND_VGA_HPP
#define _TTY_BACKEND_VGA_HPP

#include "../TTYBackend.hpp"

#include <Graphics/Colour.hpp>
#include <Graphics/VGA.hpp>

class TTYBackendVGA : public TTYBackend {
public:
    TTYBackendVGA();
    TTYBackendVGA(BasicVGA* VGADevice, const Colour& fg_colour, const Colour& bg_colour);
    ~TTYBackendVGA();

    void putc(char c) override;
    void puts(const char* str) override;
    void seek(size_t offset) override;
    size_t tell() override;

    void SetDefaultForeground(const Colour& colour);
    void SetDefaultBackground(const Colour& colour);

    BasicVGA* GetVGADevice();
    void SetVGADevice(BasicVGA* device);

    void SwapBuffers();
private:
    BasicVGA* m_VGADevice;
    Colour m_fg_colour;
    Colour m_bg_colour;
};

#endif /* _TTY_BACKEND_VGA_HPP */