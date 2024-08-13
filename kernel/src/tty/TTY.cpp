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

#include "TTY.hpp"

#include <spinlock.h>
#include <util.h>

#include <file.h>

TTY* g_CurrentTTY = nullptr;

TTY::TTY() {

}

TTY::~TTY() {

}

void TTY::Initialise() {

}

void TTY::Destroy() {

}

int TTY::getc(TTYBackendMode mode) {
    TTYBackend* backend = GetBackend(mode);
    if (backend == nullptr)
        return EOF;
    return backend->getc();
}

void TTY::putc(char c, TTYBackendMode mode) {
    TTYBackend* backend = GetBackend(mode);
    if (backend == nullptr)
        return;
    backend->putc(c);
}

void TTY::puts(const char* str, TTYBackendMode mode) {
    TTYBackend* backend = GetBackend(mode);
    if (backend == nullptr)
        return;
    backend->puts(str);
}

void TTY::seek(uint64_t offset, TTYBackendMode mode) {
    TTYBackend* backend = GetBackend(mode);
    if (backend == nullptr)
        return;
    backend->seek(offset);
}

size_t TTY::tell(TTYBackendMode mode) {
    TTYBackend* backend = GetBackend(mode);
    if (backend == nullptr)
        return 0;
    return backend->tell();
}

void TTY::SetBackend(TTYBackend* backend, TTYBackendMode mode) {
    switch (mode) {
        case TTYBackendMode::IN:
            m_backends[0].backend = backend;
            m_backends[0].mode = mode;
            break;
        case TTYBackendMode::OUT:
            m_backends[1].backend = backend;
            m_backends[1].mode = mode;
            break;
        case TTYBackendMode::ERR:
            m_backends[2].backend = backend;
            m_backends[2].mode = mode;
            break;
        case TTYBackendMode::DEBUG:
            m_backends[3].backend = backend;
            m_backends[3].mode = mode;
            break;
    }
}

TTYBackend* TTY::GetBackend(TTYBackendMode mode) {
    switch (mode) {
        case TTYBackendMode::IN:
            return m_backends[0].backend;
        case TTYBackendMode::OUT:
            return m_backends[1].backend;
        case TTYBackendMode::ERR:
            return m_backends[2].backend;
        case TTYBackendMode::DEBUG:
            return m_backends[3].backend;
    }
    return nullptr;
}

void TTY::EnumerateBackends(TTYBackendCallback callback, void* data) {
    for (size_t i = 0; i < 4; i++) {
        if (m_backends[i].backend != nullptr)
            callback(m_backends[i].mode, m_backends[i].backend, data);
    }
}

void TTY::Lock() const {
    spinlock_acquire(&m_lock);
}

void TTY::Unlock() const {
    spinlock_release(&m_lock);
}
