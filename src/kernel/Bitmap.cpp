#include "Bitmap.hpp"

namespace WorldOS {

    /* Bitmap class */

    /* Public methods */

    Bitmap::Bitmap() {
        m_Size = 0;
        m_Buffer = nullptr;
    }

    Bitmap::Bitmap(uint8_t* buffer, size_t size) {
        m_Size = size;
        m_Buffer = buffer;
    }

    Bitmap::~Bitmap() {
        m_Size = 0;
        m_Buffer = nullptr;
    }

    bool Bitmap::operator[](uint64_t index) {
        uint64_t byteIndex = index / 8;
        uint8_t bitIndex = index % 8;
        uint8_t bitIndexer = 0b10000000 >> bitIndex;
        if ((m_Buffer[byteIndex] & bitIndexer) > 0) {
            return true;
        }

        return false;
    }

    void Bitmap::Set(uint64_t index, bool value) {
        uint64_t byteIndex = index / 8;
        uint8_t bitIndex = index % 8;
        uint8_t bitIndexer = 0b10000000 >> bitIndex;
        m_Buffer[byteIndex] &= ~bitIndexer;
        if (value) {
            m_Buffer[byteIndex] |= bitIndexer;
        }
    }

}