#ifndef _TTY_BACKEND_BOCHS_DEBUG_HPP
#define _TTY_BACKEND_BOCHS_DEBUG_HPP

#include "../TTYBackend.hpp"

class TTYBackendBochsDebug : public TTYBackend {
public:
    TTYBackendBochsDebug();
    ~TTYBackendBochsDebug();

    void putc(char c) override;
    void puts(const char* str) override;
};

#endif /* _TTY_BACKEND_BOCHS_DEBUG_HPP */