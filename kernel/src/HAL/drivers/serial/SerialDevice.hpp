#ifndef _SERIAL_DEVICE_HPP
#define _SERIAL_DEVICE_HPP

#include <stdint.h>

#include "../Device.hpp"

class SerialDevice : public Device {
public:
    SerialDevice();
    ~SerialDevice();

    virtual void Initialise() = 0;
    virtual void Destroy() = 0;

    virtual int64_t Read(uint8_t* buffer, int64_t count) = 0;
    virtual int64_t Write(uint8_t* buffer, int64_t count) = 0;

    virtual const char* getVendorName() const override = 0;
    virtual const char* getDeviceName() const override = 0;
};

#endif /* _SERIAL_DEVICE_HPP */