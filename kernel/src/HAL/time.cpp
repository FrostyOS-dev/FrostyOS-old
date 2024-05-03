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

#include "time.h"

#ifdef __x86_64__
#include <arch/x86_64/RTC.hpp>
#endif

#include <util.h>
#include <stdio.h>
#include <spinlock.h>

#include "drivers/HPET.hpp"

const char* days_of_week[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char* months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

uint64_t g_timerTicks = 0;
int g_allowTimer = 0;
int g_timerRunning = 0;

void TimerCallback(void*);

extern "C" void HAL_TimeInit() {
    g_timerTicks = 0;
    g_allowTimer = 1;
    g_timerRunning = 1;
    g_HPET->StartTimer(1'000'000'000'000, TimerCallback, nullptr);
    
    RTC_Init();
    for (int i = 0; i < 5; i++) { // 5 attempts
        RTCTime time = RTC_getCurrentTime();
        sleep(1); // minimum wait time
        RTCTime time2 = RTC_getCurrentTime();
        if (time == time2) {
            dbgprintf("RTC Initialised Successfully. It is %s the %hhu of %s %hu, %.2hhu:%.2hhu:%.2hhu UTC\n", days_of_week[time.WeekDay - 1], time.DayOfMonth, months[time.Month - 1], time.Year, time.Hours, time.Minutes, time.Seconds);
            break;
        }
    }
}

extern "C" void sleep(uint64_t ms) {
    uint64_t start = GetTimer();
    while ((GetTimer() - start) < ms)
        __asm__ volatile ("" ::: "memory");
}

spinlock_new(g_timerLock);

void TimerCallback(void*) {
/*#ifdef __x86_64__
    uint64_t flags = 0;
    __asm__ volatile ("pushfq; pop %0; cli" : "=r"(flags) : : "memory");
#endif
    spinlock_acquire(&g_timerLock);*/
    if (g_allowTimer == 0) {
        g_timerRunning = 0;
        return;
    }
    __atomic_add_fetch(&g_timerTicks, 1, __ATOMIC_SEQ_CST);
    //dbgprintf("Timer tick %llu\n", g_timerTicks);
    g_HPET->StartTimer(1'000'000'000'000, TimerCallback, nullptr);
    /*spinlock_release(&g_timerLock);
#ifdef __x86_64__
    if (flags & 0x200) // if interrupts were enabled before we disabled them
        __asm__ volatile ("sti");
#endif*/
}


extern "C" uint64_t GetTimer() {
    if (g_timerRunning == 0)
        return 0;
    return g_timerTicks;
}

extern "C" time_t getTime() {
    for (int i = 0; i < 5; i++) { // 5 attempts
        RTCTime time = RTC_getCurrentTime();
        sleep(1); // minimum wait time
        RTCTime time2 = RTC_getCurrentTime();
        if (time == time2)
            return ((days_since_epoch(time.Year, time.Month, time.DayOfMonth) * 24 + time.Hours) * 60 + time.Minutes) * 60 + time.Seconds;
    }
    return 0;
}
