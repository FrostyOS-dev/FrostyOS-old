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
    if (m < 3) {
        m += 12;
        y--;
    }
    int K = y % 100;
    int J = y / 100;
    int h = (d + ((m + 1) * 26) / 10 + K + K / 4 + J / 4 + 5 * J) % 7;
    return (uint8_t)(h == 0 ? 7 : h);
}

bool g_BCD = false; // true for BCD, false for binary
bool g_12Hour = false; // true for 12-hour, false for 24-hour
bool g_RTCInitialised = false;

void RTC_Init() {
    g_BCD = !((CMOS_Read((uint8_t)RTC_Registers::STATUSB) & 4) == 4);
    g_12Hour = (CMOS_Read((uint8_t)RTC_Registers::STATUSB) & 2) == 2;
    g_RTCInitialised = true;
}

void RTC_WaitForUpdate() {
    if (!g_RTCInitialised)
        return;
    while (!((CMOS_Read((uint8_t)RTC_Registers::STATUSA) >> 7) & 1)); // wait until update starts
    while ((CMOS_Read((uint8_t)RTC_Registers::STATUSA) >> 7) & 1); // wait until update ends
}

bool RTC_IsUpdating() {
    if (!g_RTCInitialised)
        return false;
    return (CMOS_Read((uint8_t)RTC_Registers::STATUSA) >> 7) & 1;
}

/*
struct RTCTime {
    uint8_t Seconds; // 0-59
    uint8_t Minutes; // 0-59
    uint8_t Hours; // 0-23
    uint8_t WeekDay; // 1-7, Sunday = 1
    uint8_t DayOfMonth; // 1-31
    uint8_t Month; // 1-12
    uint16_t Year; // Should never have a value less than 0
} __attribute__((packed)) __attribute__((aligned(0x8))); // must fit in 64 bits
*/

RTCTime RTC_getCurrentTime() {
    if (!g_RTCInitialised) {
        return {
            .Seconds = 0,
            .Minutes = 0,
            .Hours = 0,
            .WeekDay = 5, // Thursday
            .DayOfMonth = 1,
            .Month = 1,
            .Year = 1970
        };
    }
    x86_64_DisableInterrupts();
    while (RTC_IsUpdating()); // Wait for an update to finish
    uint8_t Seconds = CMOS_Read((uint8_t)RTC_Registers::SECONDS);
    if (g_BCD)
        Seconds = BCD_TO_BINARY(Seconds);
    uint8_t Minutes = CMOS_Read((uint8_t)RTC_Registers::MINUTES);
    if (g_BCD)
        Minutes = BCD_TO_BINARY(Minutes);
    uint8_t Hours = CMOS_Read((uint8_t)RTC_Registers::HOURS);
    bool is_pm = false; // only used in 12-hour time
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

bool RTC_IsInitialised() {
    return g_RTCInitialised;
}
