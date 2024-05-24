/*
Copyright (©) 2022-2024  Frosty515

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

#ifndef _x86_64_PIT_HPP
#define _x86_64_PIT_HPP

#include <stdint.h>

#define PIT_OUT_FREQ 200
#define PIT_MS_PER_TICK 1000 / PIT_OUT_FREQ

// Init PIT. DO NOT run if hardware interrupts are enabled.
void x86_64_PIT_Init();

// Set Divisor of the PIT
void x86_64_PIT_SetDivisor(uint64_t div);

// Get Current Tick amount of the PIT
uint64_t x86_64_PIT_GetTicks();

// Set tick amount of the PIT
void x86_64_PIT_SetTicks(uint64_t t);

#endif /* _X86_64_PIT_HPP */