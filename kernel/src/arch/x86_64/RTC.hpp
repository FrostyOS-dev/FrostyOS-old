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