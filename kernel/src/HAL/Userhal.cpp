#include "hal.hpp"

#include <util.h>

void __attribute__((noreturn)) Shutdown() {
    __user_exit(0);
}

