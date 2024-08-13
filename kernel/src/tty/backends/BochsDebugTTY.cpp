#include "BochsDebugTTY.hpp"

#ifdef __x86_64__
#include <arch/x86_64/E9.h>
#endif

TTYBackendBochsDebug::TTYBackendBochsDebug() : TTYBackend(TTYBackendType::BochsDebug, TTYBackendStreamDirection::OUTPUT) {

}

TTYBackendBochsDebug::~TTYBackendBochsDebug() {

}

void TTYBackendBochsDebug::putc(char c) {
#ifdef __x86_64__
    x86_64_debug_putc(c);
#endif
}

void TTYBackendBochsDebug::puts(const char* str) {
#ifdef __x86_64__
    x86_64_debug_puts(str);
#endif
}
