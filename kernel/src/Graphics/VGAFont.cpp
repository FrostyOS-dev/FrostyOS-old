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

#include "VGAFont.hpp"

GetCharReturn getChar(const char in) {
    uint8_t out[16];
    for (uint8_t i = 0; i < 16; i++) { 
        out[i] = 0x00;
    }
    if (in < 32 || in > 126) return {out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7], out[8], out[9], out[10], out[11], out[12], out[13], out[14], out[15]};
    for (uint8_t i = 0; i < 16; i++) { 
        out[i] = letters[in-32][i];
    }
    return {out[15], out[14], out[13], out[12], out[11], out[10], out[9], out[8], out[7], out[6], out[5], out[4], out[3], out[2], out[1], out[0]};
}

