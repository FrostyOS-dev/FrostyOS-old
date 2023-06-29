#ifndef _HAL_DEVICE_HPP
#define _HAL_DEVICE_HPP

class Device {
public:
    virtual ~Device() {};

    virtual const char* getVendorName() const = 0;
    virtual const char* getDeviceName() const = 0;
};

#endif /* _HAL_DEVICE_HPP */