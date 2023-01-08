#ifndef _KERNEL_X86_64_ELF_SYMBOLS_H
#define _KERNEL_X86_64_ELF_SYMBOLS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t __text_start;
extern uint8_t __text_end;
extern uint8_t __rodata_start;
extern uint8_t __rodata_end;
extern uint8_t __data_start;
extern uint8_t __data_end;
extern uint8_t __bss_start;
extern uint8_t __bss_end;

extern const void* _text_start_addr;
extern const void* _text_end_addr;
extern const void* _rodata_start_addr;
extern const void* _rodata_end_addr;
extern const void* _data_start_addr;
extern const void* _data_end_addr;
extern const void* _bss_start_addr;
extern const void* _bss_end_addr;


#ifdef __cplusplus 
}
#endif

#endif /* _KERNEL_X86_64_ELF_SYMBOLS_H */