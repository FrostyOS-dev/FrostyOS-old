/*
Copyright (©) 2023-2024  Frosty515

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

#ifndef _SYS_MEMORY_HPP
#define _SYS_MEMORY_HPP

#define PROT_READ          1UL
#define PROT_WRITE         2UL
#define PROT_READ_WRITE    3UL
#define PROT_EXECUTE       4UL
#define PROT_READ_EXECUTE  5UL

// Request page aligned memory size bytes long, starting at addr (if null, any address) with perms. If returned address is between (void*)-1 and (void*)-100, then an error occurred and the address is the error code.
void* sys_mmap(unsigned long size, unsigned long perms, void* addr);

// Unmap page aligned memory size bytes long, starting at addr.
int sys_munmap(void* addr, unsigned long size);

// Remap page aligned memory size bytes long, starting at addr with perms.
int sys_mprotect(void* addr, unsigned long size, unsigned long perms);

#endif /* _SYS_MEMORY_HPP */