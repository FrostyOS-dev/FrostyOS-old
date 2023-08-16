/*
Copyright (©) 2023  Frosty515

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

#ifndef _HAL_DISK_HPP
#define _HAL_DISK_HPP

#include <stdint.h>
#include <stddef.h>

class Disk {
public:
    virtual ~Disk() {};

    virtual bool Read(uint8_t* buffer, uint64_t lba, uint64_t count = 1) = 0;
    virtual bool Write(const uint8_t* buffer, uint64_t lba, uint64_t count = 1) = 0;

    virtual size_t GetSectorSize() = 0;
};

#endif /* _HAL_DISK_HPP */