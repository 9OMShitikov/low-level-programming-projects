#include <stdint.h>
#include <stdarg.h>

#include "common.h"
#include "vga.h"

//prints formatted string to terminal. Supports %d, %u, %b, %o, %x, %c and %s.
int printf(const char* fmt, ...) {
    va_list argPointer;
    va_start(argPointer, fmt);
    size_t pos = 0;
    char buff[40];
    size_t buff_pos;
    while (fmt[pos] != 0) {
        if (fmt[pos] != '%') {
            terminal_putchar(fmt[pos]);
        }
        else {
            pos++;
            switch (fmt[pos]) {
                case 'd': {
                    int d = va_arg(argPointer, int);
                    if (d < 0) {
                        d = -d;
                        terminal_putchar('-');
                    }
                    buff_pos = sizeof(buff) - 1;
                    if (d == 0) {
                        terminal_putchar('0');
                    } else {
                        while (d > 0) {
                            buff[buff_pos] = d % 10 + '0';
                            d /= 10;
                            buff_pos--;
                        }
                        terminal_write_default(buff + buff_pos + 1, sizeof(buff) - buff_pos - 1);
                    }
                }
                    break;
                case 'u': {
                    uint32_t d = va_arg(argPointer, uint32_t);
                    buff_pos = sizeof(buff) - 1;
                    if (d == 0) {
                        terminal_putchar('0');
                    } else {
                        while (d > 0) {
                            buff[buff_pos] = d % 10 + '0';
                            d /= 10;
                            buff_pos--;
                        }
                        terminal_write_default(buff + buff_pos + 1, sizeof(buff) - buff_pos - 1);
                    }
                }
                    break;
                case 'b': {
                    terminal_write_default("0b", 2);
                    uint32_t d = va_arg(argPointer, uint32_t);
                    size_t len = 32;
                    for (size_t i = 0; i < len; ++i) {
                        char c = d & 1;
                        c += '0';
                        buff[len - i - 1] = c;
                        d>>=1;
                    }
                    terminal_write_default(buff, len);
                }

                    break;
                case 'o': {
                    terminal_write_default("0o", 2);
                    uint32_t d = va_arg(argPointer, uint32_t);
                    size_t len = 33/3;
                    for (size_t i = 0; i < len; ++i) {
                        char c = d % 8;
                        c += '0';
                        buff[len - i - 1] = c;
                        d /= 8;
                    }
                    terminal_write_default(buff, len);
                }
                    break;
                case 'x': {
                    terminal_write_default("0x", 2);
                    uint32_t d = va_arg(argPointer, uint32_t);
                    size_t len = 32/4;
                    for (size_t i = 0; i < len; ++i) {
                        char c = d % 16;
                        if (c <= 9) {
                            c += '0';
                        } else {
                            c += 'A'-10;
                        }
                        buff[len - i - 1] = c;
                        d /= 16;
                    }
                    terminal_write_default(buff, len);
                }
                    break;
                case 'c': {
                    int c = va_arg(argPointer, int);
                    terminal_putchar((&c)[0]);
                }
                    break;
                case 's': {
                    const char* s = va_arg(argPointer, const char*);
                    terminal_write_default(s, strlen(s));
                }
                    break;
                case '%':
                    terminal_putchar('%');
                    break;
            }
        }
        pos++;
    }
    va_end(argPointer);
    return 0;
}
