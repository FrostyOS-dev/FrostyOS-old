#ifndef _HAL_DISK_HPP
#define _HAL_DISK_HPP

#include <stdint.h>
#include <stddef.h>

class Disk {
public:
    virtual ~Disk() {};

    virtual void Read(uint8_t* buffer, uint64_t lba, uint64_t count = 1) = 0;
    virtual void Write(const uint8_t* buffer, uint64_t lba, uint64_t count = 1) = 0;

    virtual size_t GetSectorSize() = 0;
};

#endif /* _HAL_DISK_HPP */