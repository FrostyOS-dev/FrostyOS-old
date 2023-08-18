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

#include "VirtualRegion.hpp"

#include <stdio.hpp>

namespace WorldOS {

    /* VirtualRegion class */

    /* Public Methods*/

    VirtualRegion::VirtualRegion() : m_start(nullptr), m_end(nullptr), m_size(0) {

    }

    VirtualRegion::VirtualRegion(void* start, size_t size) : m_start(start), m_end((void*)((size_t)start + size)), m_size(size) {

    }

    VirtualRegion::VirtualRegion(void* start, void* end) : m_start(start), m_end(end), m_size((size_t)end - (size_t)start) {

    }

    VirtualRegion::VirtualRegion(size_t size, void* end) : m_start((void*)((size_t)end - size)), m_end(end), m_size(size) {

    }

    VirtualRegion::~VirtualRegion() {
        m_start = nullptr;
        m_end = nullptr;
        m_size = 0;
    }
    
    void* VirtualRegion::GetStart() const {
        return m_start;
    }

    void* VirtualRegion::GetEnd() const {
        return m_end;
    }

    size_t VirtualRegion::GetSize() const {
        return m_size;
    }

    void VirtualRegion::SetStart(void* start) {
        if (start > m_end)
            return; // Invalid start
        m_start = start;
        m_size = (size_t)m_end - (size_t)m_start; // Rebalance
    }

    void VirtualRegion::SetEnd(void* end) {
        if (end < m_start)
            return; // Invalid end
        m_end = end;
        m_size = (size_t)m_end - (size_t)m_start; // Rebalance
    }

    void VirtualRegion::ExpandLeft(size_t new_size) {
        m_size = new_size;
        m_start = (void*)((size_t)m_end - new_size);
    }

    void VirtualRegion::ExpandRight(size_t new_size) {
        m_size = new_size;
        m_end = (void*)((size_t)m_start + new_size);
    }

    bool VirtualRegion::IsInside(void* mem, size_t size) const {
        return (mem >= m_start) && ((void*)((size_t)mem + size) <= m_end);
    }

    bool VirtualRegion::EnsureIsInside(void*& mem, size_t& size) const {
        if (IsInside(mem, size))
            return true; // Already inside
        void* mem_i = mem;
        if (mem_i > m_end)
            return false; // Outside of bounds
        if ((void*)((size_t)mem_i + size) <= m_start)
            return false; // Outside of bounds
        if (((void*)((size_t)mem_i + size) > m_start) && (mem_i < m_start))
            mem = m_start;
        if (size > m_size)
            size -= (size_t)mem_i - (size_t)m_start;
        return true;
    }

    void VirtualRegion::fprint(fd_t file) const {
        fprintf(file, "{ m_start=%lp, m_end=%lp, m_size=%lu }\n", m_start, m_end, m_size);
    }


}