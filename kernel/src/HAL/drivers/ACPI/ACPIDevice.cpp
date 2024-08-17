#include "ACPIDevice.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <uacpi/namespace.h>
#include <uacpi/types.h>
#include <uacpi/utilities.h>
#include <uacpi/resources.h>
#pragma GCC diagnostic pop

uacpi_ns_iteration_decision match_device(void* user, uacpi_namespace_node* node) {
    DeviceCheckStatus* status = (DeviceCheckStatus*)user;
    status->node = node;
    status->found = true;
    return UACPI_NS_ITERATION_DECISION_BREAK;
}

DeviceCheckStatus DoesLegacyPITExist() {
    DeviceCheckStatus status = { "i8254 PIT", false, nullptr };
    uacpi_find_devices("PNP0100", match_device, &status);
    return status;
}

DeviceCheckStatus DoesRTCExist() {
    DeviceCheckStatus status = { "RTC", false, nullptr };
    uacpi_find_devices("PNP0B00", match_device, &status);
    return status;
}

DeviceCheckStatus DoesPS2ControllerExist() {
    DeviceCheckStatus status = { "PS/2 Controller", false, nullptr };
    uacpi_find_devices("PNP0303", match_device, &status);
    return status;
}
