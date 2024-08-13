#include "UserTTY.hpp"

#include <string.h>
#include <util.h>

UserTTY::UserTTY() : TTYBackend(TTYBackendType::User, TTYBackendStreamDirection::OUTPUT), m_hostFD(1) {

}

UserTTY::UserTTY(TTYBackendStreamDirection mode, int hostFD) : TTYBackend(TTYBackendType::User, mode), m_hostFD(hostFD) {

}

UserTTY::~UserTTY() {

}

int UserTTY::getc() {
    char c = EOF;
    long rc = __user_read(m_hostFD, &c, 1);
    if (rc < 0)
        return (int)rc;
    return c;
}

void UserTTY::putc(char c) {
    __user_write(m_hostFD, &c, 1);
}

void UserTTY::puts(const char* str) {
    __user_write(m_hostFD, str, strlen(str));
}


void UserTTY::seek(size_t offset) {
    __user_seek(m_hostFD, offset, SEEK_SET);
}

size_t UserTTY::tell() {
    return __user_seek(m_hostFD, 0, SEEK_CUR);
}
