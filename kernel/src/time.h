#ifndef _KERNEL_TIME_H
#define _KERNEL_TIME_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ISO C data types */

typedef unsigned int time_t;

/* ISO C functions */

time_t time(time_t* timer);

/* Extra functions */

int64_t years_to_days_since_epoch(int year); // Turn year into days since 01/01/1970
int64_t days_since_epoch(int year, int month, int day); // Get days since 01/01/1970

#ifdef __cplusplus
}
#endif

#endif /* _KERNEL_TIME_H */