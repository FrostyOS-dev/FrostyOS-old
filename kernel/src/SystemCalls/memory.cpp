/*
Copyright (©) 2023  Frosty515

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

#include "memory.hpp"

#include <util.h>
#include <errno.h>

#include <Scheduling/Process.hpp>
#include <Scheduling/Scheduler.hpp>

#include <Memory/PageManager.hpp>

void* sys$mmap(unsigned long size, unsigned long perms, void* addr) {
    Scheduling::Process* process = Scheduling::Scheduler::GetCurrent()->GetParent();
    using namespace WorldOS;
    PagePermissions i_perms;
    if (perms == PROT_READ)
        i_perms = PagePermissions::READ;
    else if (perms == PROT_WRITE)
        i_perms = PagePermissions::WRITE;
    else if (perms == PROT_READ_WRITE)
        i_perms = PagePermissions::READ_WRITE;
    else if (perms == PROT_EXECUTE)
        i_perms = PagePermissions::EXECUTE;
    else if (perms == PROT_READ_EXECUTE)
        i_perms = PagePermissions::READ_EXECUTE;
    else
        return (void*)-EINVAL; // read, write, execute and write, execute are unsupported
    addr = ALIGN_ADDRESS_DOWN(addr, PAGE_SIZE);
    uint64_t page_count = DIV_ROUNDUP(size, PAGE_SIZE);
    if (addr != nullptr) {
        if (!(process->GetRegion().IsInside(addr, PAGE_SIZE)))
            return (void*)-EINVAL; // bad address
    }
    void* address = process->GetPageManager()->AllocatePages(page_count, i_perms, addr);
    if (address == nullptr)
        return (void*)-ENOMEM;
    process->SyncRegion();
    return address;
}

int sys$munmap(void* addr, unsigned long size) {
    Scheduling::Process* process = Scheduling::Scheduler::GetCurrent()->GetParent();
    addr = ALIGN_ADDRESS_DOWN(addr, PAGE_SIZE);
    size = ALIGN_UP(size, PAGE_SIZE);
    if (!(process->GetRegion().IsInside(addr, size)))
        return -EINVAL; // bad address and size combination
    if (!(process->GetPageManager()->isValidAllocation(addr, size)))
        return -EINVAL; // bad address and size combination
    if ((size >> 12) == 1)
        process->GetPageManager()->FreePage(addr);
    else
        process->GetPageManager()->FreePages(addr);
    process->SyncRegion();
    return 0;
}

int sys$mprotect(void* addr, unsigned long size, unsigned long perms) {
    Scheduling::Process* process = Scheduling::Scheduler::GetCurrent()->GetParent();
    addr = ALIGN_ADDRESS_DOWN(addr, PAGE_SIZE);
    size = ALIGN_UP(size, PAGE_SIZE);
    if (!(process->GetRegion().IsInside(addr, size)))
        return -EINVAL; // bad address and size combination
    if (!(process->GetPageManager()->isValidAllocation(addr, size)))
        return -EINVAL; // bad address and size combination
    using namespace WorldOS;
    PagePermissions i_perms;
    if (perms == PROT_READ)
        i_perms = PagePermissions::READ;
    else if (perms == PROT_WRITE)
        i_perms = PagePermissions::WRITE;
    else if (perms == PROT_READ_WRITE)
        i_perms = PagePermissions::READ_WRITE;
    else if (perms == PROT_EXECUTE)
        i_perms = PagePermissions::EXECUTE;
    else if (perms == PROT_READ_EXECUTE)
        i_perms = PagePermissions::READ_EXECUTE;
    else
        return -EINVAL; // read, write, execute and write, execute are unsupported
    process->GetPageManager()->Remap(addr, i_perms);
    return 0;
}
