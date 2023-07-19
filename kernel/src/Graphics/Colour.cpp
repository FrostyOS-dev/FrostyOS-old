#include "Colour.hpp"

Colour::Colour() : m_r(0), m_g(0), m_b(0) {
    
}

Colour::Colour(uint8_t r, uint8_t g, uint8_t b) : m_r(r), m_g(g), m_b(b) {

}

Colour::~Colour() {

}

uint32_t Colour::as_ARGB() {
    return 0xFF000000 | ((uint32_t)m_r << 16) | ((uint32_t)m_g << 8) | m_b;
}

uint8_t Colour::GetRed() {
    return m_r;
}

uint8_t Colour::GetGreen() {
    return m_g;
}

uint8_t Colour::GetBlue() {
    return m_b;
}
