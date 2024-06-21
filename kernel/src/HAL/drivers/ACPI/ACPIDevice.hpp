#ifndef _HAL_ACPI_DEVICE_HPP
#define _HAL_ACPI_DEVICE_HPP

#include <uacpi/namespace.h>

struct DeviceCheckStatus {
    char const* name;
    bool found;
    uacpi_namespace_node* node;
};

DeviceCheckStatus DoesLegacyPITExist();
DeviceCheckStatus DoesRTCExist();
DeviceCheckStatus DoesPS2ControllerExist();

#endif /* _HAL_ACPI_DEVICE_HPP */