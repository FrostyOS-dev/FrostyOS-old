#ifndef _ASAN_H
#define _ASAN_H

#include <stdint.h>

enum ASANPoisonTypes {
    ASANPoison_Allocated,
    ASANPoison_Freed,
    ASANPoison_MAX = ASANPoison_Freed,
};

#ifdef __cplusplus
extern "C" {
#endif

extern const uint8_t ASANPoisonValues[ASANPoison_MAX + 1];

#ifdef __cplusplus
}
#endif

#endif /* _ASAN_H */