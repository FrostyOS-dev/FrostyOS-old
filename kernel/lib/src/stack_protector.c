/*
Copyright (©) 2023-2024  Frosty515

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

#include <stdint.h>
#include <util.h>

extern char* g_panic_reason;

#ifndef PANIC
#ifdef _FROSTYOS_BUILD_TARGET_IS_KERNEL
#ifdef __x86_64__
#define PANIC(reason) __asm__ volatile ("movq %1, %0" : "=m" (g_panic_reason) : "p" (reason)); __asm__ volatile ("call x86_64_PrePanic"); __builtin_unreachable()
#else
#error Unknown Architecture
#endif
#elif defined(_FROSTYOS_BUILD_TARGET_IS_USERLAND)
extern void __attribute__((noreturn)) UserPanic(const char* reason);
#define PANIC(reason) UserPanic(reason)
#endif /* _FROSTYOS_BUILD_TARGET_IS_KERNEL */
#endif /* PANIC */

#define STACK_CHK_GUARD 0x595e9fbd94fda766

uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

__attribute__((noreturn))
void __stack_chk_fail(void) {
    PANIC("Stack smashing detected");
}
