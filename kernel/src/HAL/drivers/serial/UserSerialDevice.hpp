#ifndef _USER_SERIAL_DEVICE_HPP
#define _USER_SERIAL_DEVICE_HPP

#include "SerialDevice.hpp"

class UserSerialDevice : public SerialDevice {
public:
    UserSerialDevice();
    ~UserSerialDevice();

    void Initialise() override;
    void Destroy() override;

    int64_t Read(uint8_t* buffer, int64_t count) override;
    int64_t Write(uint8_t* buffer, int64_t count) override;

    const char* getVendorName() const override;
    const char* getDeviceName() const override;
};

#endif /* _USER_SERIAL_DEVICE_HPP */