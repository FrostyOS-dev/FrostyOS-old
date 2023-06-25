#ifndef _KERNEL_TIME_H
#define _KERNEL_TIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL (void*)0
#endif

typedef unsigned int time_t;

time_t time(time_t* timer);

int64_t years_to_days_since_epoch(int year);
int64_t days_since_epoch(int year, int month, int day);

#ifdef __cplusplus
}
#endif

#endif /* _KERNEL_TIME_H */