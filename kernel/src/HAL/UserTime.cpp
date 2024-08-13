#include "time.h"

#include <util.h>

void sleep(uint64_t ms) {
    struct timespec ts;
    memset(&ts, 0, sizeof(ts));
    ts.tv_nsec = ms * 1'000'000;
    __user_nanosleep(&ts, nullptr);
}

time_t getTime() {
    return __user_time(nullptr);
}
