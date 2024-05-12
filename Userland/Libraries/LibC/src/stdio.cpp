/*
Copyright (©) 2022-2024  Frosty515

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

#include <stdio.h>
#include <string.h> // just used for strlen function
#include <errno.h>
#include <stdint.h>
#include <assert.h>

#include <util.h>

extern "C" {

FILE* stdin;
FILE* stdout;
FILE* stderr;
FILE* stddebug;

FILE g_stdin;
FILE g_stdout;
FILE g_stderr;
FILE g_stddebug;

FILE g_files[FOPEN_MAX];
uint8_t g_used_files; // bitmap of used files

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

void __stdio_init() {
    g_stdin = {0, O_READ};
    g_stdout = {1, O_WRITE};
    g_stderr = {2, O_WRITE};
    g_stddebug = {3, O_WRITE};

    stdin = &g_stdin;
    stdout = &g_stdout;
    stderr = &g_stderr;
    stddebug = &g_stddebug;

    fast_memset(g_files, 0, sizeof(FILE)); // we normally divide by 8, but since FOPEN_MAX is 8, we don't need to.
    g_used_files = 0;
}

}

long internal_read(FILE* file, void* data, size_t size) {
    if (file == nullptr) {
        __RETURN_WITH_ERRNO(-EFAULT);
    }
    long status = read(file->descriptor, data, size);
    if (status == EOF) {
        if (file == stdin)
            __SET_ERRNO(-ENODEV); // input device must have been disconnected
        else
            __SET_ERRNO(0);
        return EOF;
    }
    __RETURN_WITH_ERRNO(status);
}

long internal_write(FILE* file, const void* data, size_t size) {
    if (file == nullptr) {
        __RETURN_WITH_ERRNO(-EFAULT);
    }
    long status = write(file->descriptor, data, size);
    if (status == EOF) {
        __SET_ERRNO(0);
        return EOF;
    }
    __RETURN_WITH_ERRNO(status);
}

extern "C" int getc(FILE* file) {
    int c = EOF;
    long rc = internal_read(file, &c, 1); // errno already set by internal_read
    return rc == EOF ? EOF : c;
}

extern "C" int getchar() {
    int c = EOF;
    long rc = internal_read(stdin, &c, 1); // errno already set by internal_read
    return rc == EOF ? EOF : c;
}

extern "C" int fgetc(FILE* file) {
    int c = EOF;
    long rc = internal_read(file, &c, 1); // errno already set by internal_read
    return rc == EOF ? EOF : c;
}

extern "C" int putc(int c, FILE* file) {
    __RETURN_WITH_ERRNO(internal_write(file, (char*)&c, 1));
}

extern "C" int putchar(int c) {
    __RETURN_WITH_ERRNO(internal_write(stdout, (char*)&c, 1));
}

extern "C" int puts(const char* str) {
    int rc = internal_write(stdout, str, strlen(str));
    if (rc < 0) {
        __RETURN_WITH_ERRNO(rc);
    }
    rc = internal_write(stdout, "\n", 1);
    __RETURN_WITH_ERRNO(rc);
}

extern "C" int dbgputc(const char c) {
    __RETURN_WITH_ERRNO(internal_write(stddebug, (char*)&c, 1));
}

extern "C" int dbgputs(const char* str) {
    __RETURN_WITH_ERRNO(internal_write(stddebug, str, strlen(str)));
}

extern "C" int fputc(int c, FILE* file) {
    __RETURN_WITH_ERRNO(internal_write(file, (char*)&c, 1));
}

extern "C" int fputs(const char* str, FILE* file) {
    __RETURN_WITH_ERRNO(internal_write(file, str, strlen(str)));
}

enum class PRINTF_MODES {
    NORMAL,
    FLAGS,
    WIDTH,
    PREC,
    LEN,
    SPEC
};

enum class PRINTF_LENGTH {
    L_CHAR,
    L_SHORT,
    L_NORMAL,
    L_LONG,
    L_LONG_LONG,
    L_INTMAX,
    L_SIZET,
    L_PTRDIFFT,
    L_LDOUB
};

const char g_HexCharsLOWER[] = "0123456789abcdef";
const char g_HexCharsUPPER[] = "0123456789ABCDEF";

int fprintf_uint(FILE* file, int min_length, int min_digits, uint64_t num, int radix, bool padding_type, bool padding_orientation /*true for right, false for left*/, bool uppercase) {
    char buffer[64] = {0}; // max size of a valid integer. Supports a full 64-bit integer in base-2
    int64_t pos = 0;
    int chars_printed = 0;

    if (num == 0) {
        if (min_length > 1) {
            if (padding_orientation) {
                for (int i = 0; i < min_digits; i++) {
                    fputc('0', file);
                    min_length--;
                    chars_printed++;
                }
                if (min_digits != 0) {
                    fputc('0', file);
                    chars_printed++;
                    min_digits--;
                    min_length--;
                }
                for (int i = 0; i < min_length; i++) {
                    if (padding_type)
                        fputc('0', file);
                    else
                        fputc(' ', file);
                    chars_printed++;
                }
            }
            else {
                bool print_zero = false;
                if (min_digits != 0) {
                    print_zero = true;
                    min_digits--;
                    min_length--;
                }
                for (int i = 0; i < min_length; i++) {
                    if (padding_type || min_digits > 0) {
                        fputc('0', file);
                        min_digits--;
                    }
                    else
                        fputc(' ', file);
                    chars_printed++;
                }
                if (print_zero) {
                    fputc('0', file);
                    chars_printed++;
                }
            }
        }
        else {   
            if (min_digits != 0) {
                fputc('0', file);
                chars_printed++;
                min_digits--;
                min_length--;
            }
        }
        return chars_printed;
    }

    do {
        uint64_t rem = num % radix;
        num /= radix;
        if (uppercase)
            buffer[pos++] = g_HexCharsUPPER[rem];
        else
            buffer[pos++] = g_HexCharsLOWER[rem];
    } while (num > 0);


    if (min_length > pos) {
        if (padding_orientation) {
            min_length -= pos;
            min_digits -= pos;
            for (int i = 0; i < min_digits; i++) {
                fputc('0', file);
                min_length--;
                chars_printed++;
            }
            for (pos--; pos >= 0; pos--) {
                fputc(buffer[pos], file);
                chars_printed++;
            }
            for (int i = 0; i < min_length; i++) {
                if (padding_type)
                    fputc('0', file);
                else
                    fputc(' ', file);
                chars_printed++;
            }
        }
        else {
            min_length -= pos;
            min_digits -= pos;
            for (int i = 0; i < min_length; i++) {
                if (padding_type || min_digits > 0) {
                    fputc('0', file);
                    min_digits--;
                }
                else
                    fputc(' ', file);
                chars_printed++;
            }
            for (pos--; pos >= 0; pos--) {
                fputc(buffer[pos], file);
                chars_printed++;
            }
        }
    }
    else {
        pos--;
        for (; pos >= 0; pos--) {
            fputc(buffer[pos], file);
            chars_printed++;
        }
    }
    return chars_printed;
}

int fprintf_int(FILE* file, int64_t num, bool force_sign, bool pad_sign, int min_length, int min_digits, int radix, bool padding_type, bool padding_orientation /*true for right, false for left*/, bool uppercase) {
    int chars_printed = 0;
    if (num < 0) {
        fputc('-', file);
        chars_printed++;
        min_length--;
        num *= -1;
    }
    else if (force_sign) {
        fputc('+', file);
        chars_printed++;
        min_length--;
    }
    else if (pad_sign) {
        fputc(' ', file);
        chars_printed++;
        min_length--;
    }
    return fprintf_uint(file, min_length, min_digits, num, radix, padding_type, padding_orientation, uppercase) + chars_printed;
}

extern "C" int vfprintf(FILE* file, const char* format, va_list args) {
    int mode = (int)PRINTF_MODES::NORMAL;
    int radix = 10;
    int len = (int)PRINTF_LENGTH::L_NORMAL;
    bool sign = false;
    bool left_justify = false;
    bool force_sign = false;
    bool add_blank = false;
    bool flags_extra = false;
    bool zero_pad = false;
    bool gotoNextChar = false;
    bool number = false;
    bool upper = false;
    bool floating = false;
    bool use_shortest = false;
    bool scientific = false;
    int symbols_printed = 0;
    uint64_t i = 0;
    char c = format[i];
    int width = 0;
    int precision = -1;

    while (c != 0) {
        switch (mode) {
            case (int)PRINTF_MODES::NORMAL:
                switch (c) {
                    case '%':
                        mode = (int)PRINTF_MODES::FLAGS;
                        gotoNextChar = true;
                        break;
                    default:
                        fputc(c, file);
                        gotoNextChar = true;
                        symbols_printed++;
                        break;
                }
                break;
            case (int)PRINTF_MODES::FLAGS:
                switch (c) {
                    case '-':
                        left_justify = true;
                        gotoNextChar = true;
                        break;
                    case '+':
                        force_sign = true;
                        gotoNextChar = true;
                        break;
                    case ' ':
                        add_blank = true;
                        gotoNextChar = true;
                        break;
                    case '#':
                        flags_extra = true;
                        gotoNextChar = true;
                        break;
                    case '0':
                        zero_pad = true;
                        gotoNextChar = true;
                        break;
                    default:
                        gotoNextChar = false;
                        mode = (int)PRINTF_MODES::WIDTH;
                        break;
                }
                break;
            case (int)PRINTF_MODES::WIDTH:
                if (c == '*') {
                    width = va_arg(args, int);
                    mode = (int)PRINTF_MODES::LEN;
                    gotoNextChar = true;
                    break;
                }
                else if (c >= '0' && c <= '9') {
                    width *= 10;
                    width += c - '0';
                    gotoNextChar = true;
                    break;
                }
                else {
                    mode = (int)PRINTF_MODES::LEN;
                    gotoNextChar = false;
                    break;
                }
            case (int)PRINTF_MODES::PREC:
                if (c == '*') {
                    precision = va_arg(args, int);
                    mode = (int)PRINTF_MODES::LEN;
                    gotoNextChar = true;
                    break;
                }
                else if (c >= '0' && c <= '9') {
                    precision *= 10;
                    precision += c - '0';
                    gotoNextChar = true;
                    break;
                }
                else {
                    mode = (int)PRINTF_MODES::LEN;
                    gotoNextChar = false;
                    break;
                }
            case (int)PRINTF_MODES::LEN:
                switch (c) {
                    case '.':
                        if (precision == -1) { // prevent against invalid position of precision
                            mode = (int)PRINTF_MODES::PREC;
                            gotoNextChar = true;
                            precision = 0;
                            break;
                        }
                        assert(false);
                        break;
                    case 'h':
                        if (len == (int)PRINTF_LENGTH::L_SHORT)
                            len = (int)PRINTF_LENGTH::L_CHAR;
                        else
                            len = (int)PRINTF_LENGTH::L_SHORT;
                        gotoNextChar = true;
                        break;
                    case 'l':
                        if (len == (int)PRINTF_LENGTH::L_LONG)
                            len = (int)PRINTF_LENGTH::L_LONG_LONG;
                        else
                            len = (int)PRINTF_LENGTH::L_LONG;
                        gotoNextChar = true;
                        break;
                    case 'j':
                        len = (int)PRINTF_LENGTH::L_INTMAX;
                        gotoNextChar = true;
                        break;
                    case 'z':
                        len = (int)PRINTF_LENGTH::L_SIZET;
                        gotoNextChar = true;
                        break;
                    case 't':
                        len = (int)PRINTF_LENGTH::L_PTRDIFFT;
                        gotoNextChar = true;
                        break;
                    case 'L':
                        len = (int)PRINTF_LENGTH::L_LDOUB;
                        gotoNextChar = true;
                        break;
                    default:
                        mode = (int)PRINTF_MODES::SPEC;
                        gotoNextChar = false;
                        break;
                }
                break;
            case (int)PRINTF_MODES::SPEC:
                switch (c) {
                    case 'd': // signed decimal
                    case 'i':
                        radix = 10;
                        number = true;
                        sign = true;
                        floating = false;
                        break;
                    case 'u': // unsigned decimal
                        radix = 10;
                        number = true;
                        sign = false;
                        floating = false;
                        break;
                    case 'o': // unsigned octal
                        radix = 8;
                        number = true;
                        sign = false;
                        floating = false;
                        break;
                    case 'p': // pointer address
                        if (len == (int)PRINTF_LENGTH::L_NORMAL)
                            len = (int)PRINTF_LENGTH::L_LONG;
                        radix = 16;
                        number = true;
                        sign = false;
                        upper = false;
                        floating = false;
                        break;
                    case 'x': // unsigned hex (lowercase)
                        radix = 16;
                        number = true;
                        sign = false;
                        upper = false;
                        floating = false;
                        break;
                    case 'X': // unsigned hex (uppercase)
                        radix = 16;
                        number = true;
                        sign = false;
                        upper = true;
                        floating = false;
                        break;
                    case 'f': // decimal floating point (lowercase)
                        radix = 10;
                        number = true;
                        sign = true;
                        upper = false;
                        floating = true;
                        break;
                    case 'F': // decimal floating point (uppercase)
                        radix = 10;
                        number = true;
                        sign = true;
                        upper = true;
                        floating = true;
                        break;
                    case 'e': // scientific notation (lowercase)
                        radix = 10;
                        number = true;
                        sign = true;
                        floating = true;
                        upper = false;
                        scientific = true;
                        break;
                    case 'E': // scientific notation (uppercase)
                        radix = 10;
                        number = true;
                        sign = true;
                        floating = true;
                        upper = true;
                        scientific = true;
                        break;
                    case 'g': // choose 'f' or 'e', whatever is shorter
                        radix = 10;
                        number = true;
                        floating = true;
                        upper = false;
                        use_shortest = true;
                        sign = true;
                        break;
                    case 'G': // choose 'F' or 'E', whatever is shorter
                        radix = 10;
                        number = true;
                        floating = true;
                        upper = true;
                        use_shortest = true;
                        sign = true;
                        break;
                    case 'a': // Hex floating point (lowercase)
                        radix = 16;
                        upper = false;
                        floating = true;
                        sign = true;
                        number = true;
                        break;
                    case 'A': // Hex floating point (uppercase)
                        radix = 16;
                        upper = true;
                        floating = true;
                        sign = true;
                        number = true;
                        break;
                    case 'c': // character
                        fputc(va_arg(args, int /* char is promoted to int */), file);
                        symbols_printed++;
                        break;
                    case 's': // string.
                        {
                            const char* str = va_arg(args, const char*);
                            uint64_t j = 0;
                            char c2 = str[j];
                            while (c2 != 0) {
                                if (precision > 0 && j >= (unsigned int)precision)
                                    break;
                                fputc(c2, file);
                                symbols_printed++;
                                j++;
                                c2 = str[j];
                                width--;
                            }
                            for (int j = 0; j < width; j++) {
                                fputc(' ', file);
                                symbols_printed++;
                            }
                            break;
                        }
                    case 'n': // send symbols_printed to va_arg
                        switch (len) {
                            case (int)PRINTF_LENGTH::L_CHAR:
                                *(va_arg(args, signed char*)) = symbols_printed;
                                break;
                            case (int)PRINTF_LENGTH::L_SHORT:
                                *(va_arg(args, signed short int*)) = symbols_printed;
                                break;
                            case (int)PRINTF_LENGTH::L_LONG:
                                *(va_arg(args, signed long int*)) = symbols_printed;
                                break;
                            case (int)PRINTF_LENGTH::L_LONG_LONG:
                                *(va_arg(args, signed long long int*)) = symbols_printed;
                                break;
                            case (int)PRINTF_LENGTH::L_INTMAX:
                                *(va_arg(args, intmax_t*)) = symbols_printed;
                                break;
                            case (int)PRINTF_LENGTH::L_SIZET:
                                *(va_arg(args, size_t*)) = symbols_printed;
                                break;
                            case (int)PRINTF_LENGTH::L_PTRDIFFT:
                                *(va_arg(args, ptrdiff_t*)) = symbols_printed;
                                break;
                            default:
                                *(va_arg(args, signed int*)) = symbols_printed;
                                break;
                        }
                        break;
                    case '%': // print a '%'
                        fputc('%', file);
                        symbols_printed++;
                        break;
                    default: // ignore invalid spec
                        break;
                }

                if (flags_extra) {
                    switch (radix) {
                    case 16:
                        fputc('0', file);
                        fputc(upper ? 'X' : 'x', file);
                        symbols_printed += 2;
                        width -= 2;
                        if (width < 0)
                            width = 0;
                        break;
                    case 8:
                        fputc('0', file);
                        symbols_printed++;
                        width--;
                        if (width < 0)
                            width = 0;
                        break;
                    default:
                        break;
                    }
                }


                // actually do the printing
                if (number) {
                    if (floating) {
                        (void)use_shortest;
                        (void)scientific;
                        assert(false);
                    }
                    else {
                        if (sign) {
                            int64_t num = 0;
                            switch (len) {
                                case (int)PRINTF_LENGTH::L_CHAR:
                                case (int)PRINTF_LENGTH::L_SHORT:
                                    num = va_arg(args, signed int);
                                    break;
                                case (int)PRINTF_LENGTH::L_LONG:
                                    num = va_arg(args, signed long int);
                                    break;
                                case (int)PRINTF_LENGTH::L_LONG_LONG:
                                    num = va_arg(args, signed long long int);
                                    break;
                                case (int)PRINTF_LENGTH::L_INTMAX:
                                    num = va_arg(args, intmax_t);
                                    break;
                                case (int)PRINTF_LENGTH::L_SIZET:
                                    num = va_arg(args, size_t);
                                    break;
                                case (int)PRINTF_LENGTH::L_PTRDIFFT:
                                    num = va_arg(args, ptrdiff_t);
                                    break;
                                default:
                                    num = va_arg(args, signed int);
                                    break;
                            }
                            symbols_printed += fprintf_int(file, num, force_sign, add_blank, max(width, precision), precision, radix, zero_pad, left_justify, upper);
                        }
                        else {
                            uint64_t num = 0;
                            switch (len) {
                                case (int)PRINTF_LENGTH::L_CHAR:
                                case (int)PRINTF_LENGTH::L_SHORT:
                                    num = va_arg(args, unsigned int);
                                    break;
                                case (int)PRINTF_LENGTH::L_LONG:
                                    num = va_arg(args, unsigned long int);
                                    break;
                                case (int)PRINTF_LENGTH::L_LONG_LONG:
                                    num = va_arg(args, unsigned long long int);
                                    break;
                                case (int)PRINTF_LENGTH::L_INTMAX:
                                    num = va_arg(args, uintmax_t);
                                    break;
                                case (int)PRINTF_LENGTH::L_SIZET:
                                    num = va_arg(args, size_t);
                                    break;
                                case (int)PRINTF_LENGTH::L_PTRDIFFT:
                                    num = va_arg(args, ptrdiff_t);
                                    break;
                                default:
                                    num = va_arg(args, unsigned int);
                                    break;
                            }
                            symbols_printed += fprintf_uint(file, max(width, precision), precision, num, radix, zero_pad, left_justify, upper);
                        }
                    }
                }

                mode = (int)PRINTF_MODES::NORMAL;
                radix = 10;
                len = (int)PRINTF_LENGTH::L_NORMAL;
                sign = false;
                left_justify = false;
                force_sign = false;
                add_blank = false;
                flags_extra = false;
                zero_pad = false;
                number = false;
                upper = false;
                floating = false;
                use_shortest = false;
                scientific = false;
                width = 0;
                precision = -1;
                gotoNextChar = true;
                break;
            default:
                return -1; // invalid mode
        }
        if (gotoNextChar) {
            i++;
            c = format[i];
        }
    }

    return symbols_printed;
}

void snputc(char* s, size_t max_size, size_t* current_size, char c) {
    if (*current_size < max_size) {
        s[*current_size] = c;
        *current_size += 1;
    }
}

int snprintf_uint(char* s, size_t max_size, size_t* current_size, int min_length, int min_digits, uint64_t num, int radix, bool padding_type, bool padding_orientation /*true for right, false for left*/, bool uppercase) {
    char buffer[64] = {0}; // max size of a valid integer. Supports a full 64-bit integer in base-2
    int64_t pos = 0;
    int chars_printed = 0;

    if (num == 0) {
        if (min_length > 1) {
            if (padding_orientation) {
                for (int i = 0; i < min_digits; i++) {
                    snputc(s, max_size, current_size, '0');
                    min_length--;
                    chars_printed++;
                }
                if (min_digits != 0) {
                    snputc(s, max_size, current_size, '0');
                    chars_printed++;
                    min_digits--;
                    min_length--;
                }
                for (int i = 0; i < min_length; i++) {
                    if (padding_type)
                        snputc(s, max_size, current_size, '0');
                    else
                        snputc(s, max_size, current_size, ' ');
                    chars_printed++;
                }
            }
            else {
                bool print_zero = false;
                if (min_digits != 0) {
                    print_zero = true;
                    min_digits--;
                    min_length--;
                }
                for (int i = 0; i < min_length; i++) {
                    if (padding_type || min_digits > 0) {
                        snputc(s, max_size, current_size, '0');
                        min_digits--;
                    }
                    else
                        snputc(s, max_size, current_size, ' ');
                    chars_printed++;
                }
                if (print_zero) {
                    snputc(s, max_size, current_size, '0');
                    chars_printed++;
                }
            }
        }
        else {   
            if (min_digits != 0) {
                snputc(s, max_size, current_size, '0');
                chars_printed++;
                min_digits--;
                min_length--;
            }
        }
        return chars_printed;
    }

    do {
        uint64_t rem = num % radix;
        num /= radix;
        if (uppercase)
            buffer[pos++] = g_HexCharsUPPER[rem];
        else
            buffer[pos++] = g_HexCharsLOWER[rem];
    } while (num > 0);


    if (min_length > pos) {
        if (padding_orientation) {
            min_length -= pos;
            min_digits -= pos;
            for (int i = 0; i < min_digits; i++) {
                snputc(s, max_size, current_size, '0');
                min_length--;
                chars_printed++;
            }
            for (pos--; pos >= 0; pos--) {
                snputc(s, max_size, current_size, buffer[pos]);
                chars_printed++;
            }
            for (int i = 0; i < min_length; i++) {
                if (padding_type)
                    snputc(s, max_size, current_size, '0');
                else
                    snputc(s, max_size, current_size, ' ');
                chars_printed++;
            }
        }
        else {
            min_length -= pos;
            min_digits -= pos;
            for (int i = 0; i < min_length; i++) {
                if (padding_type || min_digits > 0) {
                    snputc(s, max_size, current_size, '0');
                    min_digits--;
                }
                else
                    snputc(s, max_size, current_size, ' ');
                chars_printed++;
            }
            for (pos--; pos >= 0; pos--) {
                snputc(s, max_size, current_size, buffer[pos]);
                chars_printed++;
            }
        }
    }
    else {
        pos--;
        for (; pos >= 0; pos--) {
            snputc(s, max_size, current_size, buffer[pos]);
            chars_printed++;
        }
    }
    return chars_printed;
}

int snprintf_int(char* s, size_t max_size, size_t* current_size, int64_t num, bool force_sign, bool pad_sign, int min_length, int min_digits, int radix, bool padding_type, bool padding_orientation /*true for right, false for left*/, bool uppercase) {
    int chars_printed = 0;
    if (num < 0) {
        snputc(s, max_size, current_size, '-');
        chars_printed++;
        min_length--;
        num *= -1;
    }
    else if (force_sign) {
        snputc(s, max_size, current_size, '+');
        chars_printed++;
        min_length--;
    }
    else if (pad_sign) {
        snputc(s, max_size, current_size, ' ');
        chars_printed++;
        min_length--;
    }
    return snprintf_uint(s, max_size, current_size, min_length, min_digits, num, radix, padding_type, padding_orientation, uppercase) + chars_printed;
}

extern "C" int vsnprintf(char* s, size_t n, const char* format, va_list args) {
    n--; // for the null terminating character

    int mode = (int)PRINTF_MODES::NORMAL;
    int radix = 10;
    int len = (int)PRINTF_LENGTH::L_NORMAL;
    bool sign = false;
    bool left_justify = false;
    bool force_sign = false;
    bool add_blank = false;
    bool flags_extra = false;
    bool zero_pad = false;
    bool gotoNextChar = false;
    bool number = false;
    bool upper = false;
    bool floating = false;
    bool use_shortest = false;
    bool scientific = false;
    int symbols_printed = 0;
    uint64_t i = 0;
    char c = format[i];
    int width = 0;
    int precision = -1;

    size_t current_size = 0;

    while (c != 0) {
        switch (mode) {
            case (int)PRINTF_MODES::NORMAL:
                switch (c) {
                    case '%':
                        mode = (int)PRINTF_MODES::FLAGS;
                        gotoNextChar = true;
                        break;
                    default:
                        snputc(s, n, &current_size, c);
                        gotoNextChar = true;
                        symbols_printed++;
                        break;
                }
                break;
            case (int)PRINTF_MODES::FLAGS:
                switch (c) {
                    case '-':
                        left_justify = true;
                        gotoNextChar = true;
                        break;
                    case '+':
                        force_sign = true;
                        gotoNextChar = true;
                        break;
                    case ' ':
                        add_blank = true;
                        gotoNextChar = true;
                        break;
                    case '#':
                        flags_extra = true;
                        gotoNextChar = true;
                        break;
                    case '0':
                        zero_pad = true;
                        gotoNextChar = true;
                        break;
                    default:
                        gotoNextChar = false;
                        mode = (int)PRINTF_MODES::WIDTH;
                        break;
                }
                break;
            case (int)PRINTF_MODES::WIDTH:
                if (c == '*') {
                    width = va_arg(args, int);
                    mode = (int)PRINTF_MODES::LEN;
                    gotoNextChar = true;
                    break;
                }
                else if (c >= '0' && c <= '9') {
                    width *= 10;
                    width += c - '0';
                    gotoNextChar = true;
                    break;
                }
                else {
                    mode = (int)PRINTF_MODES::LEN;
                    gotoNextChar = false;
                    break;
                }
            case (int)PRINTF_MODES::PREC:
                if (c == '*') {
                    precision = va_arg(args, int);
                    mode = (int)PRINTF_MODES::LEN;
                    gotoNextChar = true;
                    break;
                }
                else if (c >= '0' && c <= '9') {
                    precision *= 10;
                    precision += c - '0';
                    gotoNextChar = true;
                    break;
                }
                else {
                    mode = (int)PRINTF_MODES::LEN;
                    gotoNextChar = false;
                    break;
                }
            case (int)PRINTF_MODES::LEN:
                switch (c) {
                    case '.':
                        if (precision == -1) { // prevent against invalid position of precision
                            mode = (int)PRINTF_MODES::PREC;
                            gotoNextChar = true;
                            precision = 0;
                            break;
                        }
                        assert(false);
                        break;
                    case 'h':
                        if (len == (int)PRINTF_LENGTH::L_SHORT)
                            len = (int)PRINTF_LENGTH::L_CHAR;
                        else
                            len = (int)PRINTF_LENGTH::L_SHORT;
                        gotoNextChar = true;
                        break;
                    case 'l':
                        if (len == (int)PRINTF_LENGTH::L_LONG)
                            len = (int)PRINTF_LENGTH::L_LONG_LONG;
                        else
                            len = (int)PRINTF_LENGTH::L_LONG;
                        gotoNextChar = true;
                        break;
                    case 'j':
                        len = (int)PRINTF_LENGTH::L_INTMAX;
                        gotoNextChar = true;
                        break;
                    case 'z':
                        len = (int)PRINTF_LENGTH::L_SIZET;
                        gotoNextChar = true;
                        break;
                    case 't':
                        len = (int)PRINTF_LENGTH::L_PTRDIFFT;
                        gotoNextChar = true;
                        break;
                    case 'L':
                        len = (int)PRINTF_LENGTH::L_LDOUB;
                        gotoNextChar = true;
                        break;
                    default:
                        mode = (int)PRINTF_MODES::SPEC;
                        gotoNextChar = false;
                        break;
                }
                break;
            case (int)PRINTF_MODES::SPEC:
                switch (c) {
                    case 'd': // signed decimal
                    case 'i':
                        radix = 10;
                        number = true;
                        sign = true;
                        floating = false;
                        break;
                    case 'u': // unsigned decimal
                        radix = 10;
                        number = true;
                        sign = false;
                        floating = false;
                        break;
                    case 'o': // unsigned octal
                        radix = 8;
                        number = true;
                        sign = false;
                        floating = false;
                        break;
                    case 'p': // pointer address
                        if (len == (int)PRINTF_LENGTH::L_NORMAL)
                            len = (int)PRINTF_LENGTH::L_LONG;
                        radix = 16;
                        number = true;
                        sign = false;
                        upper = false;
                        floating = false;
                        break;
                    case 'x': // unsigned hex (lowercase)
                        radix = 16;
                        number = true;
                        sign = false;
                        upper = false;
                        floating = false;
                        break;
                    case 'X': // unsigned hex (uppercase)
                        radix = 16;
                        number = true;
                        sign = false;
                        upper = true;
                        floating = false;
                        break;
                    case 'f': // decimal floating point (lowercase)
                        radix = 10;
                        number = true;
                        sign = true;
                        upper = false;
                        floating = true;
                        break;
                    case 'F': // decimal floating point (uppercase)
                        radix = 10;
                        number = true;
                        sign = true;
                        upper = true;
                        floating = true;
                        break;
                    case 'e': // scientific notation (lowercase)
                        radix = 10;
                        number = true;
                        sign = true;
                        floating = true;
                        upper = false;
                        scientific = true;
                        break;
                    case 'E': // scientific notation (uppercase)
                        radix = 10;
                        number = true;
                        sign = true;
                        floating = true;
                        upper = true;
                        scientific = true;
                        break;
                    case 'g': // choose 'f' or 'e', whatever is shorter
                        radix = 10;
                        number = true;
                        floating = true;
                        upper = false;
                        use_shortest = true;
                        sign = true;
                        break;
                    case 'G': // choose 'F' or 'E', whatever is shorter
                        radix = 10;
                        number = true;
                        floating = true;
                        upper = true;
                        use_shortest = true;
                        sign = true;
                        break;
                    case 'a': // Hex floating point (lowercase)
                        radix = 16;
                        upper = false;
                        floating = true;
                        sign = true;
                        number = true;
                        break;
                    case 'A': // Hex floating point (uppercase)
                        radix = 16;
                        upper = true;
                        floating = true;
                        sign = true;
                        number = true;
                        break;
                    case 'c': // character
                        snputc(s, n, &current_size, va_arg(args, int /* char is promoted to int */));
                        symbols_printed++;
                        break;
                    case 's': // string.
                        {
                            const char* str = va_arg(args, const char*);
                            uint64_t j = 0;
                            char c2 = str[j];
                            while (c2 != 0) {
                                if (precision > 0 && j >= (unsigned int)precision)
                                    break;
                                snputc(s, n, &current_size, c2);
                                symbols_printed++;
                                j++;
                                c2 = str[j];
                                width--;
                            }
                            for (int j = 0; j < width; j++) {
                                snputc(s, n, &current_size, ' ');
                                symbols_printed++;
                            }
                            break;
                        }
                    case 'n': // send symbols_printed to va_arg
                        switch (len) {
                            case (int)PRINTF_LENGTH::L_CHAR:
                                *(va_arg(args, signed char*)) = symbols_printed;
                                break;
                            case (int)PRINTF_LENGTH::L_SHORT:
                                *(va_arg(args, signed short int*)) = symbols_printed;
                                break;
                            case (int)PRINTF_LENGTH::L_LONG:
                                *(va_arg(args, signed long int*)) = symbols_printed;
                                break;
                            case (int)PRINTF_LENGTH::L_LONG_LONG:
                                *(va_arg(args, signed long long int*)) = symbols_printed;
                                break;
                            case (int)PRINTF_LENGTH::L_INTMAX:
                                *(va_arg(args, intmax_t*)) = symbols_printed;
                                break;
                            case (int)PRINTF_LENGTH::L_SIZET:
                                *(va_arg(args, size_t*)) = symbols_printed;
                                break;
                            case (int)PRINTF_LENGTH::L_PTRDIFFT:
                                *(va_arg(args, ptrdiff_t*)) = symbols_printed;
                                break;
                            default:
                                *(va_arg(args, signed int*)) = symbols_printed;
                                break;
                        }
                        break;
                    case '%': // print a '%'
                        snputc(s, n, &current_size, '%');
                        symbols_printed++;
                        break;
                    default: // ignore invalid spec
                        break;
                }

                if (flags_extra) {
                    switch (radix) {
                    case 16:
                        snputc(s, n, &current_size, '0');
                        snputc(s, n, &current_size, upper ? 'X' : 'x');
                        symbols_printed += 2;
                        width -= 2;
                        if (width < 0)
                            width = 0;
                        break;
                    case 8:
                        snputc(s, n, &current_size, '0');
                        symbols_printed++;
                        width--;
                        if (width < 0)
                            width = 0;
                        break;
                    default:
                        break;
                    }
                }


                // actually do the printing
                if (number) {
                    if (floating) {
                        (void)use_shortest;
                        (void)scientific;
                        assert(false);
                    }
                    else {
                        if (sign) {
                            int64_t num = 0;
                            switch (len) {
                                case (int)PRINTF_LENGTH::L_CHAR:
                                case (int)PRINTF_LENGTH::L_SHORT:
                                    num = va_arg(args, signed int);
                                    break;
                                case (int)PRINTF_LENGTH::L_LONG:
                                    num = va_arg(args, signed long int);
                                    break;
                                case (int)PRINTF_LENGTH::L_LONG_LONG:
                                    num = va_arg(args, signed long long int);
                                    break;
                                case (int)PRINTF_LENGTH::L_INTMAX:
                                    num = va_arg(args, intmax_t);
                                    break;
                                case (int)PRINTF_LENGTH::L_SIZET:
                                    num = va_arg(args, size_t);
                                    break;
                                case (int)PRINTF_LENGTH::L_PTRDIFFT:
                                    num = va_arg(args, ptrdiff_t);
                                    break;
                                default:
                                    num = va_arg(args, signed int);
                                    break;
                            }
                            symbols_printed += snprintf_int(s, n, &current_size, num, force_sign, add_blank, max(width, precision), precision, radix, zero_pad, left_justify, upper);
                        }
                        else {
                            uint64_t num = 0;
                            switch (len) {
                                case (int)PRINTF_LENGTH::L_CHAR:
                                case (int)PRINTF_LENGTH::L_SHORT:
                                    num = va_arg(args, unsigned int);
                                    break;
                                case (int)PRINTF_LENGTH::L_LONG:
                                    num = va_arg(args, unsigned long int);
                                    break;
                                case (int)PRINTF_LENGTH::L_LONG_LONG:
                                    num = va_arg(args, unsigned long long int);
                                    break;
                                case (int)PRINTF_LENGTH::L_INTMAX:
                                    num = va_arg(args, uintmax_t);
                                    break;
                                case (int)PRINTF_LENGTH::L_SIZET:
                                    num = va_arg(args, size_t);
                                    break;
                                case (int)PRINTF_LENGTH::L_PTRDIFFT:
                                    num = va_arg(args, ptrdiff_t);
                                    break;
                                default:
                                    num = va_arg(args, unsigned int);
                                    break;
                            }
                            symbols_printed += snprintf_uint(s, n, &current_size, max(width, precision), precision, num, radix, zero_pad, left_justify, upper);
                        }
                    }
                }

                mode = (int)PRINTF_MODES::NORMAL;
                radix = 10;
                len = (int)PRINTF_LENGTH::L_NORMAL;
                sign = false;
                left_justify = false;
                force_sign = false;
                add_blank = false;
                flags_extra = false;
                zero_pad = false;
                number = false;
                upper = false;
                floating = false;
                use_shortest = false;
                scientific = false;
                width = 0;
                precision = -1;
                gotoNextChar = true;
                break;
            default:
                return -1; // invalid mode
        }
        if (gotoNextChar) {
            i++;
            c = format[i];
        }
    }

    snputc(s, SIZE_MAX, &current_size, 0); // max_length is irrelevant for this as we know that there is always room for a null termination.

    return symbols_printed;
}

extern "C" int fprintf(FILE* file, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vfprintf(file, format, args);
    va_end(args);
    return ret;
}

extern "C" int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vfprintf(stdout, format, args);
    va_end(args);
    return ret;
}

extern "C" int vprintf(const char* format, va_list args) {
    return vfprintf(stdout, format, args);
}

extern "C" int snprintf(char* s, size_t n, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vsnprintf(s, n, format, args);
    va_end(args);
    return ret;
}

extern "C" int sprintf(char* s, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vsnprintf(s, SIZE_MAX, format, args);
    va_end(args);
    return ret;
}

extern "C" int vsprintf(char* s, const char* format, va_list args) {
    return vsnprintf(s, SIZE_MAX, format, args);
}

extern "C" int dbgprintf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vfprintf(stddebug, format, args);
    va_end(args);
    return ret;
}

extern "C" int dbgvprintf(const char* format, va_list args) {
    return vfprintf(stddebug, format, args);
}

extern "C" size_t fwrite(const void* ptr, const size_t size, const size_t count, FILE* file) {
    for (uint64_t i = 0, current_count = 0; current_count < count; i += size, current_count++) {
        long status = internal_write(file, (void*)((uint64_t)ptr + i), size);
        if (status < 0) {
            __SET_ERRNO(status);
            return current_count;
        }
    }

    return count;
}

extern "C" size_t fread(void* ptr, const size_t size, const size_t count, FILE* file) {
    for (uint64_t i = 0, current_count = 0; current_count < count; i += size, current_count++) {
        long status = internal_read(file, (void*)((uint64_t)ptr + i), size);
        if (status < 0) {
            __SET_ERRNO(status);
            return current_count;
        }
    }

    return count;
}

uint8_t AllocateFile() {
    if (g_used_files < UINT8_MAX) {
        for (uint8_t i = 0; i < 8; i++) {
            if ((g_used_files & (1 << i)) == 0) {
                g_used_files |= 1 << i;
                return i;
            }
        }
    }
    return FOPEN_MAX;
}

bool FreeFile(FILE* file) {
    for (uint8_t i = 0; i < 8; i++) {
        if (&(file[i]) == file) {
            g_used_files &= ~(1 << i);
            return true;
        }
    }
    return false;
}

extern "C" FILE* fopen(const char* file, const char* mode) {
    if (file == nullptr || mode == nullptr) {
        __RETURN_NULL_WITH_ERRNO(-EFAULT);
    }
    
    unsigned long flags;

    if (strcmp(mode, "r") == 0)
        flags = O_READ;
    else if (strcmp(mode, "w") == 0)
        flags = O_WRITE | O_CREATE;
    else if (strcmp(mode, "a") == 0)
        flags = O_APPEND;
    else if (strcmp(mode, "r+") == 0)
        flags = O_READ | O_WRITE;
    else if (strcmp(mode, "w+") == 0)
        flags = O_READ | O_WRITE | O_CREATE;
    else { // FIXME: implement support for binary files and append/update mode
        __RETURN_NULL_WITH_ERRNO(-EINVAL);
    }

    fd_t fd = open(file, flags, 00644);
    if (fd < 0) {
        __RETURN_NULL_WITH_ERRNO(fd);
    }

    uint8_t index = AllocateFile();
    if (index == FOPEN_MAX) {
        __RETURN_NULL_WITH_ERRNO(-EMFILE);
    }

    FILE* i_file = &(g_files[index]);
    i_file->descriptor = fd;
    i_file->flags = flags;

    __SET_ERRNO(ESUCCESS);

    return i_file;
}

extern "C" int fclose(FILE* file) {
    if (file == nullptr) {
        __RETURN_WITH_ERRNO(-EFAULT);
    }
    if (!FreeFile(file)) {
        __RETURN_WITH_ERRNO(-EBADF);
    }
    __RETURN_WITH_ERRNO(close(file->descriptor));
}

extern "C" int fseek(FILE* file, long int offset, int origin) {
    if (file == nullptr) {
        __RETURN_WITH_ERRNO(-EFAULT);
    }
    __RETURN_WITH_ERRNO(seek(file->descriptor, offset, (long)origin));
}

extern "C" void rewind(FILE* file) {
    (void)fseek(file, 0, SEEK_SET);
}

extern "C" void perror(const char* str) {
    int error = errno;
    if (str != nullptr)
        fprintf(stderr, "%s: ", str);
    fprintf(stderr, "%s\n", strerror(error));
}
