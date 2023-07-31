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

#include "RTC.hpp"
#include "CMOS.h"
#include "io.h"

#include <util.h>
#include <math.h>

enum class RTC_Registers {
    SECONDS = 0x00,
    MINUTES = 0x02,
    HOURS = 0x04,
    WEEKDAY = 0x06,
    DAY_OF_MONTH = 0x07,
    MONTH = 0x08,
    YEAR = 0x09,
    STATUSA = 0x0A,
    STATUSB = 0x0B
};


bool operator==(RTCTime a, RTCTime b) {
    uint64_t* a2 = (uint64_t*)&a;
    uint64_t* b2 = (uint64_t*)&b;
    return *a2 == *b2;
}

int8_t g_null_days[12] = {1, 12, 5, 2, 7, 4, 9, 6, 3, 8, 12, 10};

uint8_t GetWeekDay(uint16_t y, uint8_t m, uint8_t d) {
    int w = (d+=m<3 ? y-- : y-2,23*m/9+d+4+y/4-y/100+y/400) % 7; // https://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week#Keith
    return (uint8_t)w + 1;
}

bool g_BCD = false; // true for BCD, false for binary
bool g_12Hour = false; // true for 12-hour, false for 24-hour
bool g_Initialised = false;

void RTC_Init() {
    g_BCD = !((CMOS_Read((uint8_t)RTC_Registers::STATUSB) & 4) == 4);
    g_12Hour = (CMOS_Read((uint8_t)RTC_Registers::STATUSB) & 2) == 2;
}

void RTC_WaitForUpdate() {
    while (!((CMOS_Read((uint8_t)RTC_Registers::STATUSA) >> 7) & 1)); // wait until update starts
    while ((CMOS_Read((uint8_t)RTC_Registers::STATUSA) >> 7) & 1); // wait until update ends
}

bool RTC_IsUpdating() {
    return (CMOS_Read((uint8_t)RTC_Registers::STATUSA) >> 7) & 1;
}

RTCTime RTC_getCurrentTime() {
    if (!g_Initialised)
        RTC_Init();
    x86_64_DisableInterrupts();
    while (RTC_IsUpdating()); // Wait for an update to finish
    uint8_t Seconds = CMOS_Read((uint8_t)RTC_Registers::SECONDS);
    if (g_BCD)
        Seconds = BCD_TO_BINARY(Seconds);
    uint8_t Minutes = CMOS_Read((uint8_t)RTC_Registers::MINUTES);
    if (g_BCD)
        Minutes = BCD_TO_BINARY(Minutes);
    uint8_t Hours = CMOS_Read((uint8_t)RTC_Registers::HOURS);
    bool is_pm; // only used in 12-hour time
    if (g_12Hour) {
        is_pm = (Hours & 0x80) == 0x80;
        Hours &= ~0x80; // Clear the bit
    }
    if (g_BCD)
        Hours = BCD_TO_BINARY(Hours);
    if (g_12Hour) {
        if (Hours == 12)
            Hours = 0;
        if (is_pm)
            Hours += 12;
    }
    uint8_t DayOfMonth = CMOS_Read((uint8_t)RTC_Registers::DAY_OF_MONTH);
    if (g_BCD)
        DayOfMonth = BCD_TO_BINARY(DayOfMonth);
    uint8_t Month = CMOS_Read((uint8_t)RTC_Registers::MONTH);
    if (g_BCD)
        Month = BCD_TO_BINARY(Month);
    uint16_t Year = CMOS_Read((uint8_t)RTC_Registers::YEAR);
    if (g_BCD)
        Year = BCD_TO_BINARY(Year);
    if (Year >= 70 && Year <= 99)
        Year += 1900;
    else
        Year += 2000; // Year must be 2000-2069
    RTCTime time = {Seconds, Minutes, Hours, GetWeekDay(Year, Month, DayOfMonth), DayOfMonth, Month, Year};
    x86_64_EnableInterrupts();
    return time;
}