#ifndef _KERNEL_BITMAP_H
#define _KERNEL_BITMAP_H

#include "stddef.h"
#include "stdint.h"

namespace WorldOS {

    class Bitmap {
    public:
        bool operator[](uint64_t index);
        void Set(uint64_t index, bool value);
    public:
        size_t Size;
        uint8_t* Buffer;
    };

}

#endif /* _KERNEL_BITMAP_H */