#ifndef _X86_64_TASK_UTIL_HPP
#define _X86_64_TASK_UTIL_HPP

#include "task.h"

#include "../interrupts/isr.hpp"

#include <stddef.h>

#include <Memory/PageManager.hpp>

#include <Scheduling/Thread.hpp>

void x86_64_PrepareNewRegisters(x86_64_Interrupt_Registers* out, const x86_64_Registers* in);
void x86_64_ConvertToStandardRegisters(x86_64_Registers* out, const x86_64_Interrupt_Registers* in);

void x86_64_GetNewStack(WorldOS::PageManager* pm, x86_64_Registers* regs, size_t size);

void x86_64_SaveIRegistersToThread(const Scheduling::Thread* thread, const x86_64_Interrupt_Registers* regs);

#endif /* _X86_64_TASK_UTIL_HPP */