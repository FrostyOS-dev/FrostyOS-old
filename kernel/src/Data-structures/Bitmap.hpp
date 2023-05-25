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

        const bool operator[](uint64_t index) const;
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