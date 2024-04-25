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

#ifndef _HAL_TIME_H
#define _HAL_TIME_H

#include <stdint.h>
#include <time.h>

#define TICKS_PER_SECOND 1000
#define MS_PER_TICK (1000 / TICKS_PER_SECOND)

#ifdef __cplusplus
extern "C" {
#endif

void HAL_TimeInit();

void sleep(uint64_t ms);

uint64_t GetTimer();

time_t getTime();

#ifdef __cplusplus
}
#endif

#endif /* _HAL_TIME_H */