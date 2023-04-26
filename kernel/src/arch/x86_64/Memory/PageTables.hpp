#ifndef _KERNEL_X86_64_PAGE_TABLES_HPP
#define _KERNEL_X86_64_PAGE_TABLES_HPP

#include <stdint.h>

struct PageMapLevel4Entry {
    uint8_t Present : 1;
    uint8_t ReadWrite : 1;
    uint8_t UserSuper : 1;
    uint8_t WriteThrough : 1;
    uint8_t CacheDisable: 1;
    uint8_t Accessed : 1;
    uint8_t Available0 : 1;
    uint8_t Reserved0 : 1;
    uint8_t Available1 : 4;
    uint64_t Address : 40;
    uint16_t Available2 : 11;
    uint8_t ExecuteDisable : 1;
} __attribute__((packed));

struct PageMapLevel3Entry {
    uint8_t Present : 1;
    uint8_t ReadWrite : 1;
    uint8_t UserSuper : 1;
    uint8_t WriteThrough : 1;
    uint8_t CacheDisable: 1;
    uint8_t Accessed : 1;
    uint8_t Available0 : 1;
    uint8_t PageSize : 1;
    uint8_t Available1 : 4;
    uint64_t Address : 40;
    uint16_t Available2 : 11;
    uint8_t ExecuteDisable : 1;
} __attribute__((packed));

struct PageMapLevel2Entry {
    uint8_t Present : 1;
    uint8_t ReadWrite : 1;
    uint8_t UserSuper : 1;
    uint8_t WriteThrough : 1;
    uint8_t CacheDisable: 1;
    uint8_t Accessed : 1;
    uint8_t Available0 : 1;
    uint8_t PageSize : 1;
    uint8_t Available1 : 4;
    uint64_t Address : 40;
    uint16_t Available2 : 11;
    uint8_t ExecuteDisable : 1;
} __attribute__((packed));

struct PageMapLevel1Entry {
    uint8_t Present : 1;
    uint8_t ReadWrite : 1;
    uint8_t UserSuper : 1;
    uint8_t WriteThrough : 1;
    uint8_t CacheDisable: 1;
    uint8_t Accessed : 1;
    uint8_t Dirty : 1;
    uint8_t PAT : 1;
    uint8_t Global : 1;
    uint8_t Available0 : 3;
    uint64_t Address : 40;
    uint8_t Available1 : 7;
    uint8_t ProctKey : 4;
    uint8_t ExecuteDisable : 1;
} __attribute__((packed));

struct CR3Layout {
    uint8_t Ignored0 : 3;
    uint8_t WriteThrough : 1;
    uint8_t CacheDisable : 1;
    uint8_t Ignored1 : 7;
    uint64_t Address : 40;
    uint16_t Reserved : 12;
} __attribute__((packed));

struct Level4Group {
    PageMapLevel4Entry entries[512];
};

struct Level3Group {
    PageMapLevel3Entry entries[512];
};

struct Level2Group {
    PageMapLevel2Entry entries[512];
};

struct Level1Group {
    PageMapLevel1Entry entries[512];
};

#endif /* _KERNEL_X86_64_PAGE_TABLES_HPP */