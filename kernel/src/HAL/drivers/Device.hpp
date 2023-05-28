#ifndef _HAL_DEVICE_HPP
#define _HAL_DEVICE_HPP

class Device {
public:
    Device();
    virtual ~Device();

    virtual const char* getVendorName();
    virtual const char* getDeviceName();
};

#endif /* _HAL_DEVICE_HPP */