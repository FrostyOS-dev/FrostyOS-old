/*
Copyright (©) 2022-2024  Frosty515

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

#ifndef _KERNEL_UTIL_H
#define _KERNEL_UTIL_H

#include <stdint.h>
#include <stddef.h>

#define KiB(x) ((uint64_t)x * (uint64_t)1024)
#define MiB(x) (KiB(x) * (uint64_t)1024)
#define GiB(x) (MiB(x) * (uint64_t)1024)

#ifndef KERNEL_STACK_SIZE
// Size of the kernel stack after init
#define KERNEL_STACK_SIZE 16384
#endif

#define volatile_read8(x) (*(volatile uint8_t*)&(x))
#define volatile_read16(x) (*(volatile uint16_t*)&(x))
#define volatile_read32(x) (*(volatile uint32_t*)&(x))
#define volatile_read64(x) (*(volatile uint64_t*)&(x))

#define volatile_write8(x, y) (*(volatile uint8_t*)&(x) = (y))
#define volatile_write16(x, y) (*(volatile uint16_t*)&(x) = (y))
#define volatile_write32(x, y) (*(volatile uint32_t*)&(x) = (y))
#define volatile_write64(x, y) (*(volatile uint64_t*)&(x) = (y))

#define _TEST_FOR_FOS_BT_kernel 1
#define _TEST_FOR_FOS_BT_Userland 2
#define _DO_TEST_FOR_FOS_BT(a, b) _DO_TEST_FOR_FOS_BT_impl(a, b)
#define _DO_TEST_FOR_FOS_BT_impl(a, b) _TEST_FOR_FOS_BT_ ## a == _TEST_FOR_FOS_BT_ ## b


#if _DO_TEST_FOR_FOS_BT(_FROSTYOS_BUILD_TARGET, kernel)
#define _FROSTYOS_BUILD_TARGET_IS_KERNEL 1
#elif _DO_TEST_FOR_FOS_BT(_FROSTYOS_BUILD_TARGET, Userland)
#define _FROSTYOS_BUILD_TARGET_IS_USERLAND 1
#endif

#undef _TEST_FOR_FOS_BT_kernel
#undef _TEST_FOR_FOS_BT_Userland
#undef _DO_TEST_FOR_FOS_BT
#undef _DO_TEST_FOR_FOS_BT_impl

#ifdef __cplusplus
extern "C" {
#endif

#define BCD_TO_BINARY(x) (((x & 0xF0) >> 1) + ((x & 0xF0) >> 3) + (x & 0xF))

#define DIV_ROUNDUP(VALUE, DIV) (VALUE + (DIV - 1)) / DIV

#define DIV_ROUNDUP_ADDRESS(ADDR, DIV) (void*)(((unsigned long)ADDR + (DIV - 1)) / DIV)

#define ALIGN_UP(VALUE, ALIGN) DIV_ROUNDUP(VALUE, ALIGN) * ALIGN

#define ALIGN_DOWN(VALUE, ALIGN) (VALUE / ALIGN) * ALIGN

#define ALIGN_ADDRESS_DOWN(ADDR, ALIGN) (void*)(((unsigned long)ADDR / ALIGN) * ALIGN)

#define ALIGN_ADDRESS_UP(ADDR, ALIGN) (void*)((((unsigned long)ADDR + (ALIGN - 1)) / ALIGN) * ALIGN)

#define PAGE_SIZE 4096

#define IN_BOUNDS(VALUE, LOW, HIGH) (LOW <= VALUE && VALUE <= HIGH)

#define TODO() __assert_fail("TODO", __FILE__, __LINE__, __ASSERT_FUNCTION)

void* memset(void* dst, const uint8_t value, const size_t n);
void* memcpy(void* dst, const void* src, const size_t n);
void* memmove(void* dst, const void* src, const size_t n);

#if _FROSTYOS_ENABLE_KASAN
void* memset_nokasan(void* dst, const uint8_t value, const size_t n);
void* memcpy_nokasan(void* dst, const void* src, const size_t n);
#else
#define memset_nokasan memset
#define memcpy_nokasan memcpy
#endif

/*
Uses 64-bit operations to quick fill a buffer.
dst is where you write to
value is the value you want to write
n is the amount of times you want to write that value
*/
void fast_memset(void* dst, const uint64_t value, const size_t n);

/*
Uses 64-bit operations to quick copy between buffers.
dst is where you write to
src is the value you want to copy from
n is the amount of data you want to copy, and must be a multiple of 8. If it isn't, it will skip the extra, so it is a multiple of 8.
*/
void* fast_memcpy(void* dst, const void* src, const size_t n);

/*
Uses 64-bit operations to quick copy between buffers.
dst is where you write to
src is the value you want to copy from
n is the amount of data you want to copy, and must be a multiple of 8. If it isn't, it will skip the extra, so it is a multiple of 8.
*/
void* fast_memmove(void* dst, const void* src, const size_t n);

int memcmp(const void* s1, const void* s2, const size_t n);
bool memcmp_b(const void* s, uint8_t c, const size_t n);

#define FLAG_SET(x, flag) x |= (flag)
#define FLAG_UNSET(x, flag) x &= ~(flag)

#ifdef _FROSTYOS_BUILD_TARGET_IS_USERLAND

#undef SEEK_SET
#undef SEEK_CUR
#undef SEEK_END

# define SEEK_SET	0	/* Seek from beginning of file.  */
# define SEEK_CUR	1	/* Seek from current position.  */
# define SEEK_END	2	/* Seek from end of file.  */

typedef unsigned int time_t;

struct timespec {
    time_t tv_sec;
    long tv_nsec;
};

struct stat {
    unsigned long st_dev;
    unsigned long st_ino;
    unsigned int st_mode;
    unsigned long st_nlink;
    unsigned int st_uid;
    unsigned int st_gid;
    unsigned long st_rdev;
    long st_size;
    long st_blksize;
    long st_blocks;

    struct timespec st_atim;
    struct timespec st_mtim;
    struct timespec st_ctim;
};

long __user_read(int fd, void* buf, size_t count);
long __user_write(int fd, const void* buf, size_t count);
int __user_open(const char* pathname, int flags);
int __user_close(int fd);
int __user_stat(const char* filename, struct stat* buf);
int __user_fstat(int fd, struct stat* buf);
long __user_seek(int fd, long offset, int whence);
void* __user_mmap(void* addr, size_t len, int prot, int flags, int fd, int offset);
int __user_mprotect(void* addr, size_t len, int prot);
int __user_munmap(void* addr, size_t len);
int __user_nanosleep(const struct timespec* req, struct timespec* rem);
long __attribute__((noreturn)) __user_exit(int status);
time_t __user_time(time_t* tloc);
#endif

#ifdef __cplusplus
}
#endif

#endif /*_KERNEL_UTIL_H */
