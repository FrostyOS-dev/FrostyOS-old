#ifndef _ACPI_HPP
#define _ACPI_HPP

int ACPI_EarlyInit(void* RSDP);
int ACPI_FullInit();

int ACPI_shutdown();
int ACPI_reboot();

#endif /* _ACPI_HPP */