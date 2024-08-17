#define UACPI_FORMATTED_LOGGING 1

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <uacpi/status.h>
#include <uacpi/kernel_api.h>
#include <uacpi/types.h>
#include <uacpi/platform/arch_helpers.h>
#pragma GCC diagnostic pop

#include <stdio.h>
#include <util.h>
#include <mutex.h>
#include <spinlock.h>
#include <stdarg.h>
#include <stddef.h>

#include <Memory/PagingUtil.hpp>
#include <Memory/kmalloc.hpp>
#include <Memory/PageManager.hpp>

#include <Scheduling/Scheduler.hpp>

#include "../../IRQ.hpp"
#include "../../time.h"

#include "../PCI.hpp"

#ifdef __x86_64__
#include <arch/x86_64/io.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Raw IO API, this is only used for accessing verified data from
 * "safe" code (aka not indirectly invoked by the AML interpreter),
 * e.g. programming FADT & FACS registers.
 *
 * NOTE:
 * 'byte_width' is ALWAYS one of 1, 2, 4, 8. You are NOT allowed to implement
 * this in terms of memcpy, as hardware expects accesses to be of the EXACT
 * width.
 * -------------------------------------------------------------------------
 */
uacpi_status uacpi_kernel_raw_memory_read(uacpi_phys_addr address, uacpi_u8 byte_width, uacpi_u64 *out_value) {
    // printf("uacpi_kernel_raw_memory_read(%lp, %u, %lp)\n", address, byte_width, out_value);
    void* virt_addr = to_HHDM((void*)address);
    switch (byte_width) {
        case 1:
            *out_value = *(uacpi_u8*)virt_addr;
            break;
        case 2:
            *out_value = *(uacpi_u16*)virt_addr;
            break;
        case 4:
            *out_value = *(uacpi_u32*)virt_addr;
            break;
        case 8:
            *out_value = *(uacpi_u64*)virt_addr;
            break;
        default:
            return UACPI_STATUS_INVALID_ARGUMENT;
    }
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_raw_memory_write(uacpi_phys_addr address, uacpi_u8 byte_width, uacpi_u64 in_value) {
    // printf("uacpi_kernel_raw_memory_write(%lp, %u, %lp)\n", address, byte_width, in_value);
    void* virt_addr = to_HHDM((void*)address);
    switch (byte_width) {
        case 1:
            *(uacpi_u8*)virt_addr = in_value;
            break;
        case 2:
            *(uacpi_u16*)virt_addr = in_value;
            break;
        case 4:
            *(uacpi_u32*)virt_addr = in_value;
            break;
        case 8:
            *(uacpi_u64*)virt_addr = in_value;
            break;
        default:
            return UACPI_STATUS_INVALID_ARGUMENT;
    }
    return UACPI_STATUS_OK;
}

/*
 * NOTE:
 * 'byte_width' is ALWAYS one of 1, 2, 4. You are NOT allowed to break e.g. a
 * 4-byte access into four 1-byte accesses. Hardware ALWAYS expects accesses to
 * be of the exact width.
 */
uacpi_status uacpi_kernel_raw_io_read(uacpi_io_addr address, uacpi_u8 byte_width, uacpi_u64 *out_value) {
    // printf("uacpi_kernel_raw_io_read(%u, %u, %lp)\n", address, byte_width, out_value);
#ifdef __x86_64__
    switch (byte_width) {
        case 1:
            *out_value = x86_64_inb(address);
            break;
        case 2:
            *out_value = x86_64_inw(address);
            break;
        case 4:
            *out_value = x86_64_ind(address);
            break;
        default:
            return UACPI_STATUS_INVALID_ARGUMENT;
    }
    return UACPI_STATUS_OK;
#else
    return UACPI_STATUS_NOT_IMPLEMENTED;
#endif
}

uacpi_status uacpi_kernel_raw_io_write(uacpi_io_addr address, uacpi_u8 byte_width, uacpi_u64 in_value) {
    // printf("uacpi_kernel_raw_io_write(%u, %u, %lp)\n", address, byte_width, in_value);
#ifdef __x86_64__
    switch (byte_width) {
        case 1:
            x86_64_outb(address, in_value);
            break;
        case 2:
            x86_64_outw(address, in_value);
            break;
        case 4:
            x86_64_outd(address, in_value);
            break;
        default:
            return UACPI_STATUS_INVALID_ARGUMENT;
    }
    return UACPI_STATUS_OK;
#else
    return UACPI_STATUS_NOT_IMPLEMENTED;
#endif
}

// -------------------------------------------------------------------------

/*
 * NOTE:
 * 'byte_width' is ALWAYS one of 1, 2, 4. Since PCI registers are 32 bits wide
 * this must be able to handle e.g. a 1-byte access by reading at the nearest
 * 4-byte aligned offset below, then masking the value to select the target
 * byte.
 */
uacpi_status uacpi_kernel_pci_read(uacpi_pci_address* in_address, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64 *value) {
    // printf("uacpi_kernel_pci_read(%lp, %u, %u, %lp)\n", in_address, offset, byte_width, value);
    uacpi_pci_address* address = (uacpi_pci_address*)to_HHDM(in_address);
    if (address->segment != 0)
        return UACPI_STATUS_UNIMPLEMENTED;
    if (address->device > 31 || address->function > 7)
        return UACPI_STATUS_INVALID_ARGUMENT;
    switch (byte_width) {
        case 1:
            PCI::PCI_ConfigReadByte(address->bus, address->device, address->function, offset, (uint8_t*)value);
            break;
        case 2:
            PCI::PCI_ConfigReadWord(address->bus, address->device, address->function, offset, (uint16_t*)value);
            break;
        case 4:
            PCI::PCI_ConfigReadDWord(address->bus, address->device, address->function, offset, (uint32_t*)value);
            break;
        default:
            return UACPI_STATUS_INVALID_ARGUMENT;
    }
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_write(uacpi_pci_address* in_address, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64 value) {
    // printf("uacpi_kernel_pci_write(%lp, %u, %u, %lp)\n", in_address, offset, byte_width, value);
    uacpi_pci_address* address = (uacpi_pci_address*)to_HHDM(in_address);
    if (address->segment != 0)
        return UACPI_STATUS_UNIMPLEMENTED;
    if (address->device > 31 || address->function > 7)
        return UACPI_STATUS_INVALID_ARGUMENT;
    switch (byte_width) {
        case 1:
            PCI::PCI_ConfigWriteByte(address->bus, address->device, address->function, offset, value);
            break;
        case 2:
            PCI::PCI_ConfigWriteWord(address->bus, address->device, address->function, offset, value);
            break;
        case 4:
            PCI::PCI_ConfigWriteDWord(address->bus, address->device, address->function, offset, value);
            break;
        default:
            return UACPI_STATUS_INVALID_ARGUMENT;
    }
    return UACPI_STATUS_OK;
}

struct io_region {
    uacpi_io_addr base;
    uacpi_size len;
};

/*
 * Map a SystemIO address at [base, base + len) and return a kernel-implemented
 * handle that can be used for reading and writing the IO range.
 */
uacpi_status uacpi_kernel_io_map(uacpi_io_addr base, uacpi_size len, uacpi_handle *out_handle) {
    // printf("uacpi_kernel_io_map(%u, %u, %lp)\n", base, len, out_handle);
    io_region* region = new io_region {
        .base = base,
        .len = len
    };
    *out_handle = region;
    return UACPI_STATUS_OK;
}

void uacpi_kernel_io_unmap(uacpi_handle handle) {
    // printf("uacpi_kernel_io_unmap(%lp)\n", handle);
    delete (io_region*)handle;
}

/*
 * Read/Write the IO range mapped via uacpi_kernel_io_map
 * at a 0-based 'offset' within the range.
 *
 * NOTE:
 * 'byte_width' is ALWAYS one of 1, 2, 4. You are NOT allowed to break e.g. a
 * 4-byte access into four 1-byte accesses. Hardware ALWAYS expects accesses to
 * be of the exact width.
 */
uacpi_status uacpi_kernel_io_read(uacpi_handle handle, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64 *value) {
    // printf("uacpi_kernel_io_read(%lp, %u, %u, %lp)\n", handle, offset, byte_width, value);
    io_region* region = (io_region*)handle;
    if (offset + byte_width > region->len)
        return UACPI_STATUS_INVALID_ARGUMENT;
#ifdef __x86_64__
    switch (byte_width) {
        case 1:
            *value = x86_64_inb(region->base + offset);
            break;
        case 2:
            *value = x86_64_inw(region->base + offset);
            break;
        case 4:
            *value = x86_64_ind(region->base + offset);
            break;
        default:
            return UACPI_STATUS_INVALID_ARGUMENT;
    }
    return UACPI_STATUS_OK;
#else
    return UACPI_STATUS_NOT_IMPLEMENTED;
#endif
}
uacpi_status uacpi_kernel_io_write(uacpi_handle handle, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64 value) {
    // printf("uacpi_kernel_io_write(%lp, %u, %u, %lp)\n", handle, offset, byte_width, value);
    io_region* region = (io_region*)handle;
    if (offset + byte_width > region->len)
        return UACPI_STATUS_INVALID_ARGUMENT;
#ifdef __x86_64__
    switch (byte_width) {
        case 1:
            x86_64_outb(region->base + offset, value);
            break;
        case 2:
            x86_64_outw(region->base + offset, value);
            break;
        case 4:
            x86_64_outd(region->base + offset, value);
            break;
        default:
            return UACPI_STATUS_INVALID_ARGUMENT;
    }
    return UACPI_STATUS_OK;
#else
    return UACPI_STATUS_NOT_IMPLEMENTED;
#endif
}

void* uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len) {
    // printf("uacpi_kernel_map(%lp, %u)\n", addr, len);
    void* out_addr = g_KPM->MapPages(ALIGN_ADDRESS_DOWN(addr, PAGE_SIZE), DIV_ROUNDUP(len, PAGE_SIZE), PagePermissions::READ_WRITE);
    if (out_addr == nullptr)
        return nullptr;
    return (void*)((size_t)out_addr + (addr & (PAGE_SIZE - 1)));
}

void uacpi_kernel_unmap(void* addr, uacpi_size len) {
    // printf("uacpi_kernel_unmap(%lp, %u)\n", addr, len);
    (void)len;
    g_KPM->UnmapPages(addr);
}

/*
 * Allocate a block of memory of 'size' bytes.
 * The contents of the allocated memory are unspecified.
 */
void *uacpi_kernel_alloc(uacpi_size size) {
    return kmalloc(size);
}

/*
 * Allocate a block of memory of 'count' * 'size' bytes.
 * The returned memory block is expected to be zero-filled.
 */
void *uacpi_kernel_calloc(uacpi_size count, uacpi_size size) {
    return kcalloc(count, size);
}

/*
 * Free a previously allocated memory block.
 *
 * 'mem' might be a NULL pointer. In this case, the call is assumed to be a
 * no-op.
 *
 * An optionally enabled 'size_hint' parameter contains the size of the original
 * allocation. Note that in some scenarios this incurs additional cost to
 * calculate the object size.
 */

void uacpi_kernel_free(void *mem) {
    kfree(mem);
}

UACPI_PRINTF_DECL(2, 3)
void uacpi_kernel_log(enum uacpi_log_level level, const char* format, ...) {
    va_list args;
    va_start(args, format);
    uacpi_kernel_vlog(level, format, args);
    va_end(args);
}

void uacpi_kernel_vlog(enum uacpi_log_level level, const char* format, uacpi_va_list args) {
    char const* prefix = "UNKNOWN";
    switch (level) {
        case UACPI_LOG_DEBUG:
            prefix = "DEBUG";
            break;
        case UACPI_LOG_TRACE:
            prefix = "TRACE";
            break;
        case UACPI_LOG_INFO:
            prefix = "INFO";
            break;
        case UACPI_LOG_WARN:
            prefix = "WARNING";
            break;
        case UACPI_LOG_ERROR:
            prefix = "ERROR";
            break;
    }
    printf("uACPI, %s: ", prefix);
    vprintf(format, args);
}

/*
 * Returns the number of 100 nanosecond ticks elapsed since boot,
 * strictly monotonic.
 */
uacpi_u64 uacpi_kernel_get_ticks(void) {
    // printf("uacpi_kernel_get_ticks()\n");
    return getNanos() * 100;
}

/*
 * Spin for N microseconds.
 */
void uacpi_kernel_stall(uacpi_u8 usec) {
    // printf("uacpi_kernel_stall(%u)\n", usec);
    uint64_t now = getNanos();
    uint64_t end = now + (uint64_t)usec * 1000;
    while (getNanos() < end);
}

/*
 * Sleep for N milliseconds.
 */
void uacpi_kernel_sleep(uacpi_u64 msec) {
    // printf("uacpi_kernel_sleep(%lu)\n", msec);
    sleep(msec);
    // printf("end uacpi_kernel_sleep(%lu)\n", msec);
}

/*
 * Create/free an opaque non-recursive kernel mutex object.
 */
uacpi_handle uacpi_kernel_create_mutex(void) {
    // printf("uacpi_kernel_create_mutex()\n");
    int rc = createMutex();
    if (rc < 0)
        return nullptr;
    return new int(rc);
}

void uacpi_kernel_free_mutex(uacpi_handle handle) {
    // printf("uacpi_kernel_free_mutex(%lp)\n", handle);
    int ID = *(int*)handle;
    delete (int*)handle;
    destroyMutex(ID);
}

/*
 * Returns a unique identifier of the currently executing thread.
 *
 * The returned thread id cannot be UACPI_THREAD_ID_NONE.
 */
uacpi_thread_id uacpi_kernel_get_thread_id(void) {
    // printf("uacpi_kernel_get_thread_id()\n");
    return Scheduling::Scheduler::GetCurrent();
}

/*
 * Create/free an opaque kernel (semaphore-like) event object.
 */
uacpi_handle uacpi_kernel_create_event(void) {
    // printf("uacpi_kernel_create_event()\n");
    return new size_t;
}

void uacpi_kernel_free_event(uacpi_handle handle) {
    // printf("uacpi_kernel_free_event(%lp)\n", handle);
    delete (size_t*)handle;
}

/*
 * Try to acquire the mutex with a millisecond timeout.
 * A timeout value of 0xFFFF implies infinite wait.
 */
uacpi_bool uacpi_kernel_acquire_mutex(uacpi_handle handle, uacpi_u16) {
    // printf("uacpi_kernel_acquire_mutex(%lp)\n", handle);
    int ID = *(int*)handle;
    return acquireMutex(ID) == 0 ? UACPI_TRUE : UACPI_FALSE;
}

void uacpi_kernel_release_mutex(uacpi_handle handle) {
    // printf("uacpi_kernel_release_mutex(%lp)\n", handle);
    int ID = *(int*)handle;
    releaseMutex(ID);
}

/*
 * Try to wait for an event (counter > 0) with a millisecond timeout.
 * A timeout value of 0xFFFF implies infinite wait.
 *
 * The internal counter is decremented by 1 if wait was successful.
 *
 * A successful wait is indicated by returning UACPI_TRUE.
 */
uacpi_bool uacpi_kernel_wait_for_event(uacpi_handle handle, uacpi_u16 timeout) {
    // printf("uacpi_kernel_wait_for_event(%lp)\n", handle);
    uint64_t now = GetTimer();
    size_t* counter = (size_t*)handle;
    do {
        if ((GetTimer() - now) > timeout && timeout != 0xFFFF)
            return UACPI_FALSE;
    } while (*counter == 0);
    (*counter)--;
    return UACPI_TRUE;
}

/*
 * Signal the event object by incrementing its internal counter by 1.
 *
 * This function may be used in interrupt contexts.
 */
void uacpi_kernel_signal_event(uacpi_handle handle) {
    // printf("uacpi_kernel_signal_event(%lp)\n", handle);
    size_t* counter = (size_t*)handle;
    (*counter)++;
}

/*
 * Reset the event counter to 0.
 */
void uacpi_kernel_reset_event(uacpi_handle handle) {
    // printf("uacpi_kernel_reset_event(%lp)\n", handle);
    size_t* counter = (size_t*)handle;
    *counter = 0;
}

/*
 * Handle a firmware request.
 *
 * Currently either a Breakpoint or Fatal operators.
 */
uacpi_status uacpi_kernel_handle_firmware_request(uacpi_firmware_request*) {
    // printf("uacpi_kernel_handle_firmware_request()\n");
    return UACPI_STATUS_UNIMPLEMENTED;
}

/*
 * Install an interrupt handler at 'irq', 'ctx' is passed to the provided
 * handler for every invocation.
 *
 * 'out_irq_handle' is set to a kernel-implemented value that can be used to
 * refer to this handler from other API.
 */
uacpi_status uacpi_kernel_install_interrupt_handler(uacpi_u32 irq, uacpi_interrupt_handler handler, uacpi_handle ctx, uacpi_handle *out_irq_handle) {
    // printf("uacpi_kernel_install_interrupt_handler(%u, %lp, %lp, %lp)\n", irq, handler, ctx, out_irq_handle);
    InterruptHandlerInfo* info = RegisterInterruptHandler(irq, handler, ctx);
    if (info == nullptr) {
        printf("Failed to register interrupt handler for IRQ %u\n", irq);
        return UACPI_STATUS_INVALID_ARGUMENT;
    }
    *out_irq_handle = info;
    return UACPI_STATUS_OK;
}

/*
 * Uninstall an interrupt handler. 'irq_handle' is the value returned via
 * 'out_irq_handle' during installation.
 */
uacpi_status uacpi_kernel_uninstall_interrupt_handler(uacpi_interrupt_handler, uacpi_handle irq_handle) {
    // printf("uacpi_kernel_uninstall_interrupt_handler(%lp)\n", irq_handle);
    UnregisterInterruptHandler((InterruptHandlerInfo*)irq_handle);
    return UACPI_STATUS_OK;
}

/*
 * Create/free a kernel spinlock object.
 *
 * Unlike other types of locks, spinlocks may be used in interrupt contexts.
 */
uacpi_handle uacpi_kernel_create_spinlock(void) {
    spinlock_t* lock = new spinlock_t;
    spinlock_init(lock);
    return lock;
}

void uacpi_kernel_free_spinlock(uacpi_handle handle) {
    delete (spinlock_t*)handle;
}

/*
 * Lock/unlock helpers for spinlocks.
 *
 * These are expected to disable interrupts, returning the previous state of cpu
 * flags, that can be used to possibly re-enable interrupts if they were enabled
 * before.
 *
 * Note that lock is infalliable.
 */
uacpi_cpu_flags uacpi_kernel_spinlock_lock(uacpi_handle handle) {
    uacpi_cpu_flags flags;
#ifdef __x86_64__
    __asm__ volatile("pushfq;pop %0;cli" : "=r"(flags));
#else
    flags = 0;
#endif
    spinlock_t* lock = (spinlock_t*)handle;
    spinlock_acquire(lock);
    return flags;
}

void uacpi_kernel_spinlock_unlock(uacpi_handle handle, uacpi_cpu_flags flags) {
    spinlock_t* lock = (spinlock_t*)handle;
    spinlock_release(lock);
#ifdef __x86_64__
    if (flags & 0x200)
        __asm__ volatile("sti");
#endif
}

/*
 * Schedules deferred work for execution.
 * Might be invoked from an interrupt context.
 */
uacpi_status uacpi_kernel_schedule_work(uacpi_work_type type, uacpi_work_handler handler, uacpi_handle ctx) {
    // printf("uacpi_kernel_schedule_work(%u, %lp, %lp)\n", ctx);
    return UACPI_STATUS_UNIMPLEMENTED;
}

/*
 * Blocks until all scheduled work is complete and the work queue becomes empty.
 */
uacpi_status uacpi_kernel_wait_for_work_completion(void) {
    // printf("uacpi_kernel_wait_for_work_completion()\n");
    return UACPI_STATUS_UNIMPLEMENTED;
}

#ifdef __cplusplus
}
#endif