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

#ifndef _KERNEL_X86_64_ELF_SYMBOLS_HPP
#define _KERNEL_X86_64_ELF_SYMBOLS_HPP

#include <stdint.h>
#include <stddef.h>


extern "C" {

extern uint8_t __text_start;
extern uint8_t __text_end;
extern uint8_t __rodata_start;
extern uint8_t __rodata_end;
extern uint8_t __data_start;
extern uint8_t __data_end;
extern uint8_t __bss_start;
extern uint8_t __bss_end;
extern uint8_t __kernel_end;

extern const void* _text_start_addr;
extern const void* _text_end_addr;
extern const void* _rodata_start_addr;
extern const void* _rodata_end_addr;
extern const void* _data_start_addr;
extern const void* _data_end_addr;
extern const void* _bss_start_addr;
extern const void* _bss_end_addr;
extern const void* _kernel_end_addr;
 
}


#include <Data-structures/LinkedList.hpp>

struct ELFSymbol {
    char const* name;
    uint64_t address;
};

class ELFSymbols {
public:
    ELFSymbols();
    ELFSymbols(const void* data, size_t size, bool eternal = false);
    ~ELFSymbols();

    const char* LookupSymbol(uint64_t address);
    uint64_t LookupSymbol(const char* name);

private:
    bool m_eternal;
    LinkedList::SimpleLinkedList<ELFSymbol> m_symbols;
};

extern ELFSymbols* g_KernelSymbols;



#endif /* _KERNEL_X86_64_ELF_SYMBOLS_HPP */