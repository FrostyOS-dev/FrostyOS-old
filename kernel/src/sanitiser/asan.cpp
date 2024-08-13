#include "Memory/kmalloc.hpp"
#include "util.h"
#include <stdio.h>
#include <stdint.h>

#include "sanitiser.h"

#define ENUMERATE_ASAN_ERROR_TYPES(E)\
E(InvalidAccess, "Invalid Access") \
E(ShadowSpaceAccess, "Shadow Space Access") \
E(UseAfterFree, "Use After Free") \
E(INVALID, "Invalid")

enum class ASANErrorType {
#define __ENUMERATE_TYPE(n, s) n,
    ENUMERATE_ASAN_ERROR_TYPES(__ENUMERATE_TYPE)
#undef __ENUMERATE_TYPE
};

enum ASANPoisonTypes {
    ASANPoison_Allocated,
    ASANPoison_Freed,
    ASANPoison_MAX = ASANPoison_Freed,
};

extern "C" const uint8_t ASANPoisonValues[ASANPoison_MAX + 1] = {
    0xAA,
    0xBB,
};

__attribute__((no_sanitize("address","kernel-address"))) const char* asan_get_error_name(ASANErrorType type) {
    switch (type) {
#define __ENUMERATE_TYPE(n, s) \
case ASANErrorType::n: \
    return s;
    ENUMERATE_ASAN_ERROR_TYPES(__ENUMERATE_TYPE)
#undef __ENUMERATE_TYPE
    default:
        return "Invalid";
    }
}

__attribute__((no_sanitize("address","kernel-address"))) void asan_report(uint64_t addr, uint64_t size, uint64_t ip, bool rw, ASANErrorType type, bool abort) {
    char buffer[1024]; // should need that many
    snprintf(buffer, 1023, "ASAN Violation! %s at %lp whilst trying to %s %lu bytes from %lp", asan_get_error_name(type), ip, rw ? "write" : "read", size, addr);
    sanitiser_panic(buffer);
}

__attribute__((no_sanitize("address","kernel-address"))) void asan_shadow_space_access(uint64_t addr, uint64_t size, uint64_t ip, bool rw, uint64_t poison_type, bool abort) {
    // Must verify if the address is in the heap or not
    size_t sizeInHeap = kmalloc_SizeInHeap((void*)addr, size);
    //dbgprintf("ASAN: Checking address %lp, size = %lu, rw = %s, poison_type = %lu. sizeInHeap = %lu\n", addr, size, rw ? "write" : "read", poison_type, sizeInHeap);
    if (sizeInHeap == 0)
        return;
    // There must be at least 16 bytes of the poison value on at least one side of the address
    bool poisoned = false;
    if (sizeInHeap >= (16 + size)) // can check after
        poisoned = memcmp_b((void*)((uint64_t)addr + size), ASANPoisonValues[poison_type], 16);
    else if (kmalloc_SizeInHeap((void*)((uint64_t)addr - 16), 16) == 16)
        poisoned = memcmp_b((void*)((uint64_t)addr - 16), ASANPoisonValues[poison_type], 16);
    if (poisoned) {
        ASANErrorType type = ASANErrorType::INVALID;
        switch (poison_type) {
        case ASANPoison_Allocated:
            type = ASANErrorType::ShadowSpaceAccess;
            break;
        case ASANPoison_Freed:
            type = ASANErrorType::UseAfterFree;
            break;
        default:
            break;
        }
        asan_report(addr, size, ip, rw, type, abort);
    }
}

__attribute__((no_sanitize("address","kernel-address"))) void asan_verify(uint64_t addr, uint64_t size, uint64_t ip, bool rw, bool abort) {
#ifdef __x86_64__
    if (!((addr >> 47) == 0 || (addr >> 47) == 0x1ffff))
        asan_report(addr, size, ip, rw, ASANErrorType::InvalidAccess, abort);
#endif
    if (memcmp_b((void*)addr, ASANPoisonValues[ASANPoison_Allocated], size))
        asan_shadow_space_access(addr, size, ip, rw, ASANPoison_Allocated, abort);
    if (memcmp_b((void*)addr, ASANPoisonValues[ASANPoison_Freed], size))
        asan_shadow_space_access(addr, size, ip, rw, ASANPoison_Freed, abort);
}

#define ASAN_LOAD_ABORT(size) \
extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_load##size(uint64_t addr) { \
    asan_verify(addr, size, (uint64_t)__builtin_extract_return_addr(__builtin_return_address(0)), false, true); \
}

#define ASAN_LOAD_NOABORT(size) \
extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_load##size##_noabort(uint64_t addr) { \
    asan_verify(addr, size, (uint64_t)__builtin_extract_return_addr(__builtin_return_address(0)), false, false); \
}

#define ASAN_STORE_ABORT(size) \
extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_store##size(uint64_t addr) { \
    asan_verify(addr, size, (uint64_t)__builtin_extract_return_addr(__builtin_return_address(0)), true, true); \
}

#define ASAN_STORE_NOABORT(size) \
extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_store##size##_noabort(uint64_t addr) { \
    asan_verify(addr, size, (uint64_t)__builtin_extract_return_addr(__builtin_return_address(0)), true, false); \
}

ASAN_LOAD_ABORT(1)
ASAN_LOAD_NOABORT(1)
ASAN_STORE_ABORT(1)
ASAN_STORE_NOABORT(1)

ASAN_LOAD_ABORT(2)
ASAN_LOAD_NOABORT(2)
ASAN_STORE_ABORT(2)
ASAN_STORE_NOABORT(2)

ASAN_LOAD_ABORT(4)
ASAN_LOAD_NOABORT(4)
ASAN_STORE_ABORT(4)
ASAN_STORE_NOABORT(4)

ASAN_LOAD_ABORT(8)
ASAN_LOAD_NOABORT(8)
ASAN_STORE_ABORT(8)
ASAN_STORE_NOABORT(8)

ASAN_LOAD_ABORT(16)
ASAN_LOAD_NOABORT(16)
ASAN_STORE_ABORT(16)
ASAN_STORE_NOABORT(16)

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_loadN(uint64_t addr, uint64_t size) {
    asan_verify(addr, size, (uint64_t)__builtin_extract_return_addr(__builtin_return_address(0)), false, true);
}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_loadN_noabort(uint64_t addr, uint64_t size) {
    asan_verify(addr, size, (uint64_t)__builtin_extract_return_addr(__builtin_return_address(0)), false, false);
}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_storeN(uint64_t addr, uint64_t size) {
    asan_verify(addr, size, (uint64_t)__builtin_extract_return_addr(__builtin_return_address(0)), true, true);
}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_storeN_noabort(uint64_t addr, uint64_t size) {
    asan_verify(addr, size, (uint64_t)__builtin_extract_return_addr(__builtin_return_address(0)), true, false);
}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_load_n(uint64_t addr, uint64_t size) {
    asan_verify(addr, size, (uint64_t)__builtin_extract_return_addr(__builtin_return_address(0)), false, true);
}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_load_n_noabort(uint64_t addr, uint64_t size) {
    asan_verify(addr, size, (uint64_t)__builtin_extract_return_addr(__builtin_return_address(0)), false, false);
}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_store_n(uint64_t addr, uint64_t size) {
    asan_verify(addr, size, (uint64_t)__builtin_extract_return_addr(__builtin_return_address(0)), true, true);
}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_store_n_noabort(uint64_t addr, uint64_t size) {
    asan_verify(addr, size, (uint64_t)__builtin_extract_return_addr(__builtin_return_address(0)), true, false);
}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_handle_no_return() {
    // Do nothing
}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_register_globals() {

}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_unregister_globals() {

}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_version_mismatch_check_v8() {

}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_init() {

}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_option_detect_stack_use_after_return() {

}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_stack_malloc_0() {

}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_stack_malloc_1() {

}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_stack_malloc_2() {

}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_stack_malloc_3() {

}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_stack_malloc_4() {

}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_stack_malloc_5() {

}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_stack_free_0() {

}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_stack_free_1() {

}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_stack_free_2() {

}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_stack_free_3() {

}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_stack_free_4() {

}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_stack_free_5() {

}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_before_dynamic_init() {
    
}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_after_dynamic_init() {

}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_poison_stack_memory() {
    
}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void __asan_unpoison_stack_memory() {
    
}