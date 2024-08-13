#ifndef _TTY_BACKEND_USER_HPP
#define _TTY_BACKEND_USER_HPP

#include "../TTYBackend.hpp"

class UserTTY : public TTYBackend {
public:
    UserTTY();
    UserTTY(TTYBackendStreamDirection mode, int hostFD);
    ~UserTTY();

    int getc() override;
    void putc(char c) override;
    void puts(const char* str) override;

    void seek(size_t offset) override;
    size_t tell() override;

private:
    int m_hostFD;
};

#endif /* _TTY_BACKEND_USER_HPP */