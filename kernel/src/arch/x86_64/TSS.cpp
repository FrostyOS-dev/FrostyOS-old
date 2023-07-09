#include "TSS.hpp"
#include "Stack.h"
#include "GDT.hpp"

#include <util.h>

x86_64_TSS g_TSS;

void x86_64_TSS_Init() {
    fast_memset(&g_TSS, 0, sizeof(g_TSS) / 8);
    g_TSS.RSP[0] = (uint64_t)kernel_stack + kernel_stack_size;
    x86_64_GDT_SetTSS(&g_TSS);
}
