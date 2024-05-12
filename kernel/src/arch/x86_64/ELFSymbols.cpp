/*
Copyright (©) 2022-2024  Frosty515

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

#include "ELFSymbols.hpp"

extern "C" {

const void* _text_start_addr   = &__text_start;
const void* _text_end_addr     = &__text_end;
const void* _rodata_start_addr = &__rodata_start;
const void* _rodata_end_addr   = &__rodata_end;
const void* _data_start_addr   = &__data_start;
const void* _data_end_addr     = &__data_end;
const void* _bss_start_addr    = &__bss_start;
const void* _bss_end_addr      = &__bss_end;
const void* _kernel_end_addr   = &__kernel_end;

}

#include <string.h>
#include <stdlib.h>
#include <util.h>

#include <HAL/hal.hpp>

ELFSymbols::ELFSymbols() : m_eternal(false) {

}

__attribute__((no_sanitize("undefined"))) ELFSymbols::ELFSymbols(const void* data, size_t size, bool eternal) : m_eternal(eternal), m_symbols(eternal) {
    if (size < 10)
        return; // too small
    if (data == nullptr)
        return; // invalid buffer
    void* (*calloc)(size_t, size_t) = nullptr;
    if (m_eternal)
        calloc = kcalloc_eternal;
    else
        calloc = kcalloc;
    const uint8_t* i_data = (const uint8_t*)data;
    uint64_t index = 0;
    while (index < size) {
        uint64_t const* p_address = reinterpret_cast<const uint64_t*>(&(i_data[index]));
        index += 8;
        if (i_data[index] || (index + 1) >= size)
            return; // invalid format
        index++;
        char const* name = reinterpret_cast<const char*>(&(i_data[index]));
        size_t name_length = strlen(name);
        char* new_name = (char*)calloc(name_length + 1, sizeof(char));
        if (new_name == nullptr)
            return;
        memcpy(new_name, name, name_length);
        ELFSymbol* symbol = (ELFSymbol*)calloc(1, sizeof(ELFSymbol));
        symbol->address = *p_address;
        symbol->name = new_name;
        m_symbols.insert(symbol);
        index += name_length + 1;
    }
}

ELFSymbols::~ELFSymbols() {
    if (m_eternal) {
        PANIC("Eternal ELF Symbol table destruction requested.");
    }
    for (uint64_t i = m_symbols.getCount(); i > 0; i++) {
        ELFSymbol* symbol = m_symbols.get(i - 1);
        if (symbol == nullptr)
            return;
        delete symbol->name;
        delete symbol;
        m_symbols.remove(i - 1);
    }
}

const char* ELFSymbols::LookupSymbol(uint64_t address) {
    for (uint64_t i = 0; i < m_symbols.getCount(); i++) {
        ELFSymbol* symbol = m_symbols.get(i);
        if (symbol == nullptr)
            return nullptr;
        if (symbol->address == address)
            return symbol->name;
        ELFSymbol* next = m_symbols.get(i + 1);
        if (next != nullptr) {
            if (symbol->address <= address && next->address > address)
                return symbol->name;
        }
    }
    return nullptr;
}

uint64_t ELFSymbols::LookupSymbol(const char* name) {
    for (uint64_t i = 0; i < m_symbols.getCount(); i++) {
        ELFSymbol* symbol = m_symbols.get(i);
        if (symbol == nullptr)
            return 0;
        if (strcmp(name, symbol->name) == 0)
            return symbol->address;
    }
    return 0;
}

ELFSymbols* g_KernelSymbols = nullptr;
