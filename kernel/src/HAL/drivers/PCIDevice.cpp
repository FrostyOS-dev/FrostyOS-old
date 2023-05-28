#include "PCIDevice.hpp"

#include <Memory/kmalloc.hpp>

#include <string.h>

PCIDevice::PCIDevice() : m_device(nullptr) {

}

PCIDevice::~PCIDevice() {

}

void PCIDevice::InitPCIDevice(PCI::Header0* device) {
    m_device = device;
}

const char* PCIDevice::getVendorName() {
    char* ret = nullptr;

    if (m_device == nullptr) {
        ret = (char*)kmalloc(17);
        if (ret == nullptr)
            return nullptr;
        strcpy(ret, "(Unknown vendor)");
        return ret;
    }


    switch (m_device->ch.VendorID) {
        case 0x8086: // Intel Corporation
            ret = (char*)kmalloc(18);
            if (ret == nullptr)
                return nullptr;
            strcpy(ret, "Intel Corporation");
            break;
        default: // (Unknown vendor)
            ret = (char*)kmalloc(17);
            if (ret == nullptr)
                return nullptr;
            strcpy(ret, "(Unknown vendor)");
            break;
    }
    return ret;
}

const char* PCIDevice::getDeviceName() {
    char* ret = nullptr;

    if (m_device == nullptr) {
        ret = (char*)kmalloc(17);
        if (ret == nullptr)
            return nullptr;
        strcpy(ret, "(Unknown device)");
        return ret;
    }


    switch (m_device->ch.VendorID) {
        case 0x8086: // Intel Corporation
            switch (m_device->ch.DeviceID) {
                default: // (Unknown device)
                    ret = (char*)kmalloc(17);
                    if (ret == nullptr)
                        return nullptr;
                    strcpy(ret, "(Unknown device)");
            break;
            }
            break;
        default: // (Unknown vendor)
            ret = (char*)kmalloc(17);
            if (ret == nullptr)
                return nullptr;
            strcpy(ret, "(Unknown device)"); // Unable to find vendor so device can't be found
            break;
    }
    return ret;
}

const char* PCIDevice::getDeviceClass() {
    char* ret = (char*)kmalloc(23);
    if (ret == nullptr)
        return nullptr;
    strcpy(ret, "(Unknown device class)");
    return ret;
}

const char* PCIDevice::getDeviceSubClass() {
    char* ret = (char*)kmalloc(26);
    if (ret == nullptr)
        return nullptr;
    strcpy(ret, "(Unknown device subclass)");
    return ret;
}

const char* PCIDevice::getDeviceProgramInterface() {
    char* ret = (char*)kmalloc(35);
    if (ret == nullptr)
        return nullptr;
    strcpy(ret, "(Unknown device program interface)");
    return ret;
}
