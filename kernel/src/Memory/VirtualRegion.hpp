#ifndef _KERNEL_VIRTUAL_MEMORY_REGION_HPP
#define _KERNEL_VIRTUAL_MEMORY_REGION_HPP

#include <stdio.hpp>
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

        bool IsInside(void* mem, size_t size = 1) const;

        bool EnsureIsInside(void*& mem, size_t& size) const;

        void fprint(fd_t file) const;

    private:
        void* m_start;
        void* m_end;
        size_t m_size;
    };

}

#endif /* _KERNEL_VIRTUAL_MEMORY_REGION_HPP */