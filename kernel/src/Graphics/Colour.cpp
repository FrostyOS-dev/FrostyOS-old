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

#include "Colour.hpp"

ColourFormat::ColourFormat() : m_bpp(0), m_red_shift(0), m_green_shift(0), m_blue_shift(0), m_red_mask(0), m_green_mask(0), m_blue_mask(0) {

}

ColourFormat::ColourFormat(uint16_t bpp, uint8_t red_shift, uint8_t red_size, uint8_t green_shift, uint8_t green_size, uint8_t blue_shift, uint8_t blue_size) : m_bpp(bpp), m_red_shift(red_shift), m_green_shift(green_shift), m_blue_shift(blue_shift), m_red_mask(0), m_green_mask(0), m_blue_mask(0) {
    m_red_mask = (1 << red_size) - 1;
    m_green_mask = (1 << green_size) - 1;
    m_blue_mask = (1 << blue_size) - 1;
}

ColourFormat::~ColourFormat() {

}

uint64_t ColourFormat::render(uint8_t r, uint8_t g, uint8_t b) const {
    return (((r & m_red_mask) << m_red_shift) | ((g & m_green_mask) << m_green_shift) | ((b & m_blue_mask) << m_blue_shift)) & ((UINT64_C(1) << m_bpp) - 1);
}



Colour::Colour() : m_r(0), m_g(0), m_b(0), m_render(0), m_format() {
    
}

Colour::Colour(const ColourFormat& format, uint8_t r, uint8_t g, uint8_t b) : m_r(r), m_g(g), m_b(b), m_render(0), m_format(format) {

}

Colour::~Colour() {

}

uint32_t Colour::as_ARGB() {
    return 0xFF000000 | ((uint32_t)m_r << 16) | ((uint32_t)m_g << 8) | m_b;
}

uint8_t Colour::GetRed() {
    return m_r;
}

uint8_t Colour::GetGreen() {
    return m_g;
}

uint8_t Colour::GetBlue() {
    return m_b;
}

uint64_t Colour::render() const {
    if (m_render == 0)
        m_render = m_format.render(m_r, m_g, m_b); // only render once
    return m_render;
}

const ColourFormat& Colour::GetFormat() const {
    return m_format;
}
