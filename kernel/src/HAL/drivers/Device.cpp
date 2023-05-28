#include "Device.hpp"

#include <string.h>

#include <Memory/kmalloc.hpp>

Device::Device() {

}

Device::~Device() {

}

const char* Device::getVendorName() {
    char* ret = (char*)kmalloc(17);
    if (ret == nullptr)
        return nullptr;
    strcpy(ret, "(Unknown vendor)");
    return ret;
}

const char* Device::getDeviceName() {
    char* ret = (char*)kmalloc(17);
    if (ret == nullptr)
        return nullptr;
    strcpy(ret, "(Unknown device)");
    return ret;
}
