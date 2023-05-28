#include "MCFG.hpp"

ACPISDTHeader* g_MCFG;

bool InitAndValidateMCFG(ACPISDTHeader* MCFG) {
    if (MCFG == nullptr)
        return false;
    if (doChecksum(MCFG)) {
        g_MCFG = MCFG;
        return true;
    }
    return false;
}

MCFGEntry* GetMCFGEntry(uint64_t index) {
    MCFGEntry* start = (MCFGEntry*)((uint64_t)g_MCFG + sizeof(ACPISDTHeader) + 8/* Reserved */);
    return (MCFGEntry*)&(start[index]);
}

uint64_t GetMCFGEntryCount() {
    return (g_MCFG->Length - sizeof(ACPISDTHeader) - 8/* Reserved */) / sizeof(MCFGEntry);
}
