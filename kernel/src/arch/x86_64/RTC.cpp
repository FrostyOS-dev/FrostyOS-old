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


uint8_t g_null_days[12] = {1, 12, 5, 2, 7, 4, 9, 6, 3, 8, 12, 10};

uint8_t GetWeekDay(uint16_t Year, uint8_t m, uint8_t d) {
    m -= 1; // Get to a better offset
    if (m > 11)
        m = 11; // ensure it is not out of bounds
    int y0 = Year % 10;
    int y1 = (Year / 10) % 10;
    int c = Year / 100;
    int d0 = g_null_days[m];
    int w = (d - d0 + y0 - y1 + ((y0 / 4) - (y1 / 2)) - 2 * (c % 4)) % 7;
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