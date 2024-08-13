/*
Copyright (©) 2022-2024  Frosty515

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _KERNEL_TTY_HPP
#define _KERNEL_TTY_HPP

#include <util.h>

#include "TTYBackend.hpp"
#include "spinlock.h"

enum class TTYBackendMode {
    IN,
    OUT,
    ERR,
    DEBUG
};

typedef void (*TTYBackendCallback)(TTYBackendMode mode, TTYBackend* backend, void* data);

class TTY {
public:
    TTY();
    ~TTY();

    void Initialise();
    void Destroy();

    int getc(TTYBackendMode mode = TTYBackendMode::IN);
    void putc(char c, TTYBackendMode mode = TTYBackendMode::OUT);
    void puts(const char* str, TTYBackendMode mode = TTYBackendMode::OUT);
    void seek(uint64_t offset, TTYBackendMode mode = TTYBackendMode::OUT);
    size_t tell(TTYBackendMode mode = TTYBackendMode::OUT);

    void SetBackend(TTYBackend* backend, TTYBackendMode mode);
    TTYBackend* GetBackend(TTYBackendMode mode);

    void EnumerateBackends(TTYBackendCallback callback, void* data);

    void Lock() const;
    void Unlock() const;

private:
    struct TTYBackendData {
        TTYBackend* backend;
        TTYBackendMode mode;
    };

    TTYBackendData m_backends[4];

    mutable spinlock_t m_lock;
};

extern TTY* g_CurrentTTY;

#endif /* _KERNEL_TTY_HPP */