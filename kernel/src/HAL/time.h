#ifndef _HAL_TIME_H
#define _HAL_TIME_H

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

void HAL_TimeInit();

void sleep(uint64_t ms);

time_t getTime();

#ifdef __cplusplus
}
#endif

#endif /* _HAL_TIME_H */