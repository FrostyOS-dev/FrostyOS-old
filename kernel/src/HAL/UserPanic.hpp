#ifndef _HAL_USERPANIC_HPP
#define _HAL_USERPANIC_HPP

#include <util.h>

#ifdef _FROSTYOS_BUILD_TARGET_IS_USERLAND

#define PANIC(reason) UserPanic(reason)

extern "C" void __attribute__((noreturn)) UserPanic(const char* reason);

#endif /* _FROSTYOS_BUILD_TARGET_IS_USERLAND */

#endif /* _HAL_USERPANIC_HPP */