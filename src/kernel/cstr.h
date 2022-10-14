#ifndef _KERNEL_CSTR_H
#define _KERNEL_CSTR_H

#include "stddef.h"
#include "stdint.h"
#include "Memory/Memory.h"

const char* to_string(uint64_t value);
const char* to_string(int64_t value);
//const char* to_string(double value, uint8_t decimalPlaces = 2);

inline const char* to_string(uint32_t value) { return to_string((uint64_t)value); }
inline const char* to_string(int32_t value) { return to_string((int64_t)value); }
inline const char* to_string(uint16_t value) { return to_string((uint64_t)value); }
inline const char* to_string(int16_t value) { return to_string((int64_t)value); }
inline const char* to_string(uint8_t value) { return to_string((uint64_t)value); }
inline const char* to_string(int8_t value) { return to_string((int64_t)value); }
//inline const char* to_string(float value) { return to_string((double)value); }

const char* to_hstring(uint64_t value);

size_t strlen(const char* str);

#endif /* _KERNEL_CSTR_H */