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

#ifndef _X86_64_RTC_HPP
#define _X86_64_RTC_HPP

#include <stdint.h>

struct RTCTime {
    uint8_t Seconds; // 0-59
    uint8_t Minutes; // 0-59
    uint8_t Hours; // 0-23
    uint8_t WeekDay; // 1-7, Sunday = 1
    uint8_t DayOfMonth; // 1-31
    uint8_t Month; // 1-12
    uint16_t Year; // Should never have a value less than 0
} __attribute__((packed)) __attribute__((aligned(0x8))); // must fit in 64 bits

bool operator==(RTCTime a, RTCTime b);

void RTC_Init();
void RTC_WaitForUpdate();
RTCTime RTC_getCurrentTime();

#endif /* _X86_64_RTC_HPP */