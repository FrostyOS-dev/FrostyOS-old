#ifndef _HAL_PCI_DEVICE_HPP
#define _HAL_PCI_DEVICE_HPP

#include "Device.hpp"
#include "PCI.hpp"

class PCIDevice : public Device {
public:
    virtual ~PCIDevice() override {}

    virtual void InitPCIDevice(PCI::Header0* device) = 0;

    virtual const char* getDeviceClass() const = 0;
    virtual const char* getDeviceSubClass() const = 0;
    virtual const char* getDeviceProgramInterface() const = 0;

protected:
    PCI::Header0* p_device = nullptr;
};

#endif /* _HAL_PCI_DEVICE_HPP */