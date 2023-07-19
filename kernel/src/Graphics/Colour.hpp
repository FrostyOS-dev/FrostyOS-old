#ifndef _KERNEL_COLOUR_HPP
#define _KERNEL_COLOUR_HPP

#include <stdint.h>

class Colour {
public:
    Colour();
    Colour(uint8_t r, uint8_t g, uint8_t b);
    ~Colour();

    uint32_t as_ARGB();
    uint8_t GetRed();
    uint8_t GetGreen();
    uint8_t GetBlue();

private:
    uint8_t m_r, m_g, m_b;
};

#endif /* _KERNEL_COLOUR_HPP */