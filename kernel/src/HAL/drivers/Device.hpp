#ifndef _HAL_DEVICE_HPP
#define _HAL_DEVICE_HPP

class Device {
public:
    virtual ~Device() {};

    virtual const char* getVendorName() = 0;
    virtual const char* getDeviceName() = 0;
};

#endif /* _HAL_DEVICE_HPP */