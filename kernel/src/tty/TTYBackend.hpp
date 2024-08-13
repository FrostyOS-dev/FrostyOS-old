#ifndef _TTY_BACKEND_HPP
#define _TTY_BACKEND_HPP

#include <stddef.h>
#include <file.h>

enum class TTYBackendType {
    VGA,
    Serial,
    BochsDebug,
    KeyboardInput,
    User
};

enum class TTYBackendStreamDirection {
    INPUT,
    OUTPUT,
    INPUT_OUTPUT
};

class TTYBackend {
public:
    virtual ~TTYBackend() = default;

    virtual int getc() { return EOF; }
    virtual void putc(char c) {}
    virtual void puts(const char* str) {}

    virtual void seek(size_t offset) {}
    virtual size_t tell() { return 0; }

    TTYBackendType getType() const { return m_type; }
    TTYBackendStreamDirection getMode() const { return m_mode; }

protected:
    TTYBackend(TTYBackendType type, TTYBackendStreamDirection mode) : m_type(type), m_mode(mode) {}

private:
    TTYBackendType m_type;
    TTYBackendStreamDirection m_mode;
};

#endif /* _TTY_BACKEND_HPP */