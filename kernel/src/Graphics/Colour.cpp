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

Colour::Colour() : m_r(0), m_g(0), m_b(0) {
    
}

Colour::Colour(uint8_t r, uint8_t g, uint8_t b) : m_r(r), m_g(g), m_b(b) {

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
