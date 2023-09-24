/*
Copyright (©) 2022-2023  Frosty515

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

#ifndef _KERNEL_VIRTUAL_MEMORY_REGION_HPP
#define _KERNEL_VIRTUAL_MEMORY_REGION_HPP

#include <stdio.h>
#include <stddef.h>

namespace WorldOS {
    class VirtualRegion {
    public:
        VirtualRegion();
        VirtualRegion(void* start, size_t size);
        VirtualRegion(void* start, void* end);
        VirtualRegion(size_t size, void* end);
        ~VirtualRegion();

        void* GetStart() const;
        void* GetEnd() const;
        size_t GetSize() const;

        void SetStart(void* start);
        void SetEnd(void* end);

        void ExpandLeft(size_t new_size); // Adjust the start to compensate for the new size
        void ExpandRight(size_t new_size); // Adjust the end to compensate for the new size

        bool IsInside(const void* mem, size_t size = 1) const;

        bool EnsureIsInside(void*& mem, size_t& size) const;

        void fprint(fd_t file) const;

    private:
        void* m_start;
        void* m_end;
        size_t m_size;
    };

}

#endif /* _KERNEL_VIRTUAL_MEMORY_REGION_HPP */