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

#ifndef _KERNEL_BITMAP_HPP
#define _KERNEL_BITMAP_HPP

#include <stddef.h>
#include <stdint.h>

namespace WorldOS {

    class Bitmap {
    public:
        Bitmap();
        Bitmap(uint8_t* buffer, size_t size);
        ~Bitmap();

        bool operator[](uint64_t index) const;
        void Set(uint64_t index, bool value);

        // Set size in bytes
        inline void SetSize(size_t size) {
            m_Size = size;
        }

        inline void SetBuffer(uint8_t* buffer) {
            m_Buffer = buffer;
        }

        // Get size in bytes
        inline size_t GetSize() const {
            return m_Size;
        }

        inline uint8_t* GetBuffer() const {
            return m_Buffer;
        }

    private:
        size_t m_Size;
        uint8_t* m_Buffer;
    };

}

#endif /* _KERNEL_BITMAP_HPP */