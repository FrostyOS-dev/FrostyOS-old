#ifndef _KERNEL_X86_64_E9_H
#define _KERNEL_X86_64_E9_H

#ifdef __cplusplus
extern "C" {
#endif

void x86_64_debug_putc(const char c);
void x86_64_debug_puts(const char* str);

#ifdef __cplusplus
}
#endif

#endif /* _KERNEL_X86_64_E9_H */