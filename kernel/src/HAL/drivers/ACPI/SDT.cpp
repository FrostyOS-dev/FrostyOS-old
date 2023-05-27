#include "SDT.hpp"

bool doChecksum(ACPISDTHeader* h) {
    unsigned char sum = 0;
 
    for (int i = 0; i < h->Length; i++)
    {
        sum += ((char*)h)[i];
    }
 
    return sum == 0;
}
