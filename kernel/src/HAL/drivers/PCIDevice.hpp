#ifndef _HAL_PCI_DEVICE_HPP
#define _HAL_PCI_DEVICE_HPP

#include "Device.hpp"
#include "PCI.hpp"

class PCIDevice : public Device {
public:
    PCIDevice();
    virtual ~PCIDevice() override;

    virtual void InitPCIDevice(PCI::Header0* device);

    virtual const char* getVendorName() override;
    virtual const char* getDeviceName() override;

    virtual const char* getDeviceClass();
    virtual const char* getDeviceSubClass();
    virtual const char* getDeviceProgramInterface();

protected:
    PCI::Header0* m_device;
};

#endif /* _HAL_PCI_DEVICE_HPP */