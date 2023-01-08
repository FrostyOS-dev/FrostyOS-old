#include "stdio.hpp"

#include <stdbool.h>

#include <string.h> // just used for strlen function

void fputc(const fd_t file, const char c) {
    VFS_write(file, (uint8_t*)&c, 1);
}

void fputs(const fd_t file, const char* str) {
    VFS_write(file, (uint8_t*)str, strlen(str));
}

void putc(const char c) {
    fputc(VFS_STDOUT, c);
}

void puts(const char* str) {
    fputs(VFS_STDOUT, str);
}

#define PRINTF_STATE_NORMAL         0
#define PRINTF_STATE_LENGTH         1
#define PRINTF_STATE_LENGTH_SHORT   2
#define PRINTF_STATE_LENGTH_LONG    3
#define PRINTF_STATE_SPEC           4

#define PRINTF_LENGTH_DEFAULT       0
#define PRINTF_LENGTH_SHORT_SHORT   1
#define PRINTF_LENGTH_SHORT         2
#define PRINTF_LENGTH_LONG          3
#define PRINTF_LENGTH_LONG_LONG     4

const char g_HexChars[] = "0123456789abcdef";

void fprintf_unsigned(const fd_t file, uint64_t number, uint8_t radix) {
    char buffer[32];
    int pos = 0;

    // convert number to ASCII
    do 
    {
        unsigned long long rem = number % radix;
        number /= radix;
        buffer[pos++] = g_HexChars[rem];
    } while (number > 0);

    // print number in reverse order
    while (--pos >= 0)
        fputc(file, buffer[pos]);
}

void fprintf_signed(const fd_t file, int64_t number, uint8_t radix) {
    if (number < 0) {
        fputc(file, '-');
        fprintf_unsigned(file, -number, radix);
    }
    else fprintf_unsigned(file, number, radix);
}

void vfprintf(const fd_t file, const char* format, va_list args) {
    int state = PRINTF_STATE_NORMAL;
    int length = PRINTF_LENGTH_DEFAULT;
    uint8_t radix = 10;
    bool sign = false;
    bool number = false;
    bool gotoNextChar = true;

    uint64_t i = 0;

    char c = format[i];

    while (c != 0) {
        switch (state) {
            case PRINTF_STATE_NORMAL:
                switch (c) {
                    case '%':
                        state = PRINTF_STATE_LENGTH;
                        gotoNextChar = true;
                        break;
                    default:
                        fputc(file, c);
                        gotoNextChar = true;
                        break;
                }
                break;

            case PRINTF_STATE_LENGTH:
                switch (c) {
                    case 'h':
                        length = PRINTF_LENGTH_SHORT;
                        state = PRINTF_STATE_LENGTH_SHORT;
                        gotoNextChar = true;
                        break;
                    case 'l':
                        length = PRINTF_LENGTH_LONG;
                        state = PRINTF_STATE_LENGTH_LONG;
                        gotoNextChar = true;
                        break;
                    default:
                        state = PRINTF_STATE_SPEC;
                        gotoNextChar = false;
                        break;
                }
                break;

            case PRINTF_STATE_LENGTH_SHORT:
                if (c == 'h') {
                    length = PRINTF_LENGTH_SHORT_SHORT;
                    gotoNextChar = true;
                }
                else gotoNextChar = false;
                state = PRINTF_STATE_SPEC;
                break;

            case PRINTF_STATE_LENGTH_LONG:
                if (c == 'h') {
                    length = PRINTF_LENGTH_LONG_LONG;
                    gotoNextChar = true;
                }
                else gotoNextChar = false;
                state = PRINTF_STATE_SPEC;
                break;

            case PRINTF_STATE_SPEC:
                switch (c) {
                    case 'c':
                        fputc(file, (char)va_arg(args, int));
                        break;

                    case 's':   
                        fputs(file, va_arg(args, const char*));
                        break;

                    case '%':
                        fputc(file, '%');
                        break;

                    case 'd':
                    case 'i':
                        radix = 10; sign = true; number = true;
                        break;

                    case 'u':
                        radix = 10; sign = false; number = true;
                        break;

                    case 'X':
                    case 'x':
                    case 'p':
                        radix = 16; sign = false; number = true;
                        break;

                    case 'o':
                        radix = 8; sign = false; number = true;
                        break;

                    // ignore invalid spec
                    default:
                        break;
                }

                // actually print arg
                if (number) {
                    if (sign) {
                        switch (length) {
                        case PRINTF_LENGTH_SHORT_SHORT:
                        case PRINTF_LENGTH_SHORT:
                        case PRINTF_LENGTH_DEFAULT:     fprintf_signed(file, va_arg(args, int), radix);
                                                        break;

                        case PRINTF_LENGTH_LONG:        fprintf_signed(file, va_arg(args, long), radix);
                                                        break;

                        case PRINTF_LENGTH_LONG_LONG:   fprintf_signed(file, va_arg(args, long long), radix);
                                                        break;
                        }
                    }
                    else {
                        switch (length) {
                        case PRINTF_LENGTH_SHORT_SHORT:
                        case PRINTF_LENGTH_SHORT:
                        case PRINTF_LENGTH_DEFAULT:     fprintf_unsigned(file, va_arg(args, int), radix);
                                                        break;

                        case PRINTF_LENGTH_LONG:        fprintf_unsigned(file, va_arg(args, long), radix);
                                                        break;

                        case PRINTF_LENGTH_LONG_LONG:   fprintf_unsigned(file, va_arg(args, long long), radix);
                                                        break;
                        }
                    }
                }

                state = PRINTF_STATE_NORMAL;
                length = PRINTF_LENGTH_DEFAULT;
                radix = 10;
                sign = false;
                number = false;
                gotoNextChar = true;
                break;
        }
        if (gotoNextChar) {
            i++;
            c = format[i];
        }
    }
}

void fprintf(const fd_t file, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(file, format, args);
    va_end(args);
}

void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(VFS_STDOUT, format, args);
    va_end(args);
}

void vprintf(const char* format, va_list args) {
    vfprintf(VFS_STDOUT, format, args);
}

void fwrite(const void* ptr, const size_t size, const size_t count, const fd_t file) {
    uint8_t* out = (uint8_t*)ptr;

    for (uint64_t i = 0; i < count; i+=size) {
        VFS_write(file, (uint8_t*)((uint64_t)out * i), size);
    }

    out = nullptr; /* just in case contents of out (which points to ptr) wants to be cleared */
}
