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