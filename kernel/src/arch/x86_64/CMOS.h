#ifndef _X86_64_CMOS_H
#define _X86_64_CMOS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t CMOS_Read(uint8_t reg);
extern void CMOS_Write(uint8_t reg, uint8_t value);

#ifdef __cplusplus
}
#endif

#endif /* _X86_64_CMOS_H */