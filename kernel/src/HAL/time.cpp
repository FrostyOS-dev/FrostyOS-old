#include "time.h"

#include <arch/x86_64/PIT.hpp>
#include <arch/x86_64/RTC.hpp>

#include <util.h>
#include <stdio.hpp>

const char* days_of_week[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char* months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

extern "C" void HAL_TimeInit() {
    x86_64_PIT_Init();
    x86_64_PIT_SetDivisor(11932 /* just slightly under 100 Hz */);
    RTC_Init();
    for (int i = 0; i < 5; i++) { // 5 attempts
        RTCTime time = RTC_getCurrentTime();
        sleep(10); // minimum wait time
        RTCTime time2 = RTC_getCurrentTime();
        if (time == time2) {
            fprintf(VFS_DEBUG, "RTC Initialised Successfully. It is %s the %hhu of %s %hu, %hhu:%hhu:%hhu UTC\n", days_of_week[time.WeekDay - 1], time.DayOfMonth, months[time.Month - 1], time.Year, time.Hours, time.Minutes, time.Seconds);
            break;
        }
    }
}

extern "C" void sleep(uint64_t ms) {
    x86_64_PIT_SetTicks(0);
    ms = ALIGN_UP(ms, 10);

    while (x86_64_PIT_GetTicks() < ms) {}
        //__asm__ volatile ("hlt"); // does the same as having an empty loop except the CPU uses less power
}

extern "C" time_t getTime() {
    for (int i = 0; i < 5; i++) { // 5 attempts
        RTCTime time = RTC_getCurrentTime();
        sleep(10); // minimum wait time
        RTCTime time2 = RTC_getCurrentTime();
        if (time == time2)
            return ((days_since_epoch(time.Year, time.Month, time.DayOfMonth) * 24 + time.Hours) * 60 + time.Minutes) * 60 + time.Seconds;
    }
    return 0;
}
