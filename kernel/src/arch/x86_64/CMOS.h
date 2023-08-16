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

#ifndef _X86_64_CMOS_H
#define _X86_64_CMOS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t CMOS_Read(uint8_t reg);
extern void CMOS_Write(uint8_t reg, uint8_t value);

#ifdef __cplusplus
}
#endif

#endif /* _X86_64_CMOS_H */