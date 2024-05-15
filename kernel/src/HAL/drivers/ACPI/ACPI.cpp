#include "ACPI.hpp"
#include "uacpi/sleep.h"
#include "uacpi/status.h"

#include <uacpi/uacpi.h>
#include <uacpi/event.h>

#include <stdio.h>
#include <errno.h>

int ACPI_init(void* RSDP) {
    uacpi_phys_addr rsdp_phys = (uacpi_phys_addr)RSDP;
    uacpi_init_params init_params = {
        .rsdp = rsdp_phys,
        .rt_params = {
            .log_level = UACPI_LOG_TRACE,
            .flags = 0
        }
    };

    uacpi_status rc = uacpi_initialize(&init_params);
    if (uacpi_unlikely_error(rc)) {
        dbgprintf("Failed to initialize uacpi: %s\n", uacpi_status_to_string(rc));
        return -ENODEV;
    }

    rc = uacpi_namespace_load();
    if (uacpi_unlikely_error(rc)) {
        dbgprintf("Failed to load ACPI namespace: %s\n", uacpi_status_to_string(rc));
        return -ENODEV;
    }

    rc = uacpi_namespace_initialize();
    if (uacpi_unlikely_error(rc)) {
        dbgprintf("Failed to initialize ACPI namespace: %s\n", uacpi_status_to_string(rc));
        return -ENODEV;
    }

    rc = uacpi_finalize_gpe_initialization();
    if (uacpi_unlikely_error(rc)) {
        dbgprintf("Failed to finalize GPE initialization: %s\n", uacpi_status_to_string(rc));
        return -ENODEV;
    }

    return 0;
}

int ACPI_shutdown() {
    uacpi_status rc = uacpi_prepare_for_sleep_state(UACPI_SLEEP_STATE_S5);
    if (uacpi_unlikely_error(rc)) {
        dbgprintf("Failed to prepare for S5: %s\n", uacpi_status_to_string(rc));
        return -EIO;
    }

#ifdef __x86_64__
    __asm__ volatile ("cli");
#endif

    rc = uacpi_enter_sleep_state(UACPI_SLEEP_STATE_S5);
    if (uacpi_unlikely_error(rc)) {
        dbgprintf("Failed to enter S5: %s\n", uacpi_status_to_string(rc));
        return -EIO;
    }

    return 0; // should be unreachable.
}