#include "FADT.hpp"
#include "uacpi/tables.h"

#include <stdio.h>
#include <uacpi/acpi.h>

acpi_fadt* g_FADT;

bool InitFADT() {
    acpi_fadt* fadt_data = nullptr;
    uacpi_status rc = uacpi_table_fadt(&fadt_data);
    if (rc != UACPI_STATUS_OK) {
        printf("Failed to get FADT table: %s\n", uacpi_status_to_string(rc));
        return false;
    }

    g_FADT = fadt_data;

    return true;
}
