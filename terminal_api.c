#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include "terminal_api.h"

uint8_t default_color = VGA_COLOR_LIGHT_GREY | VGA_COLOR_BLACK << 4;

size_t terminal_row;
size_t terminal_column;
uint16_t* terminal_buffer;

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    uint8_t color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t*) 0xB8000;
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', color);
        }
    }
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_scroll(size_t dist) {
    if (dist < VGA_HEIGHT) {
        for (size_t i = 0; i < VGA_HEIGHT - dist; ++i) {
            for (size_t j = 0; j < VGA_WIDTH; ++j) {
                terminal_buffer[i * VGA_WIDTH + j] = terminal_buffer[(i + 1) * VGA_WIDTH + j];
            }
        }
    }
    size_t start = 0;
    if (dist < VGA_HEIGHT) {
        start = VGA_HEIGHT - dist;
    }
    uint16_t d_entry = vga_entry(' ', default_color);
    for (size_t i = start; i < VGA_HEIGHT; ++i) {
        for (size_t j = 0; j < VGA_WIDTH; ++j) {
            terminal_buffer[i * VGA_WIDTH + j] = d_entry;
        }
    }
    if (dist < terminal_row) {
        terminal_row -= dist;
    }
    else {
        terminal_row = 0;
    }
}

void terminal_putchar_color(char c, uint8_t color) {
    switch (c) {
        case '\n':
            terminal_row++;
            terminal_column = 0;
            if (terminal_row == VGA_HEIGHT) {
                terminal_scroll(1);
            }
            break;

        default:
            terminal_putentryat(c, color, terminal_column, terminal_row);
            terminal_column++;
            if (terminal_column == VGA_WIDTH) {
                terminal_column = 0;
                terminal_row++;
                if (terminal_row == VGA_HEIGHT) {
                    terminal_scroll(1);
                }
            }
    }
}

void terminal_write(const char* data, size_t size, uint8_t color) {
    for (size_t i = 0; i < size; i++) {
        terminal_putchar_color(data[i], color);
    }
}

void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data), vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
}

void terminal_writestring_color(const char* data, uint8_t color) {
    terminal_write(data, strlen(data), color);
}

int terminal_printf(const char* fmt, ...) {
    va_list argPointer;
    va_start(argPointer, fmt);
    size_t pos = 0;
    char buff[40];
    size_t buff_pos;
    while (fmt[pos] != 0) {
        if (fmt[pos] != '%') {
            terminal_putchar_color(fmt[pos], default_color);
        }
        else {
            pos++;
            switch (fmt[pos]) {
                case 'd': {
                    int d = va_arg(argPointer, int);
                    if (d < 0) {
                        d = -d;
                        terminal_putchar_color('-', default_color);
                    }
                    buff_pos = sizeof(buff) - 1;
                    if (d == 0) {
                        terminal_putchar_color('0', default_color);
                    } else {
                        while (d > 0) {
                            buff[buff_pos] = d % 10 + '0';
                            d /= 10;
                            buff_pos--;
                        }
                        terminal_write(buff + buff_pos + 1, sizeof(buff) - buff_pos - 1, default_color);
                    }
                }
                    break;
                case 'u': {
                    uint32_t d = va_arg(argPointer, uint32_t);
                    buff_pos = sizeof(buff) - 1;
                    if (d == 0) {
                        terminal_putchar_color('0', default_color);
                    } else {
                        while (d > 0) {
                            buff[buff_pos] = d % 10 + '0';
                            d /= 10;
                            buff_pos--;
                        }
                        terminal_write(buff + buff_pos + 1, sizeof(buff) - buff_pos - 1, default_color);
                    }
                }
                    break;
                case 'b': {
                    terminal_write("0b", 2, default_color);
                    uint32_t d = va_arg(argPointer, uint32_t);
                    size_t len = 32;
                    for (size_t i = 0; i < len; ++i) {
                        char c = d & 1;
                        c += '0';
                        buff[len - i - 1] = c;
                        d>>=1;
                    }
                    terminal_write(buff, len, default_color);
                }

                    break;
                case 'o': {
                    terminal_write("0o", 2, default_color);
                    uint32_t d = va_arg(argPointer, uint32_t);
                    size_t len = 33/3;
                    for (size_t i = 0; i < len; ++i) {
                        char c = d % 8;
                        c += '0';
                        buff[len - i - 1] = c;
                        d /= 8;
                    }
                    terminal_write(buff, len, default_color);
                }
                    break;
                case 'x': {
                        terminal_write("0x", 2, default_color);
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
                        terminal_write(buff, len, default_color);
                    }
                    break;
                case 'c': {
                    int c = va_arg(argPointer, int);
                    terminal_putchar_color((&c)[0], default_color);
                }
                    break;
                case 's': {
                    const char* s = va_arg(argPointer, const char*);
                    terminal_write(s, strlen(s), default_color);
                }
                    break;
                case '%':
                    terminal_putchar_color('%', default_color);
                    break;
            }
        }
        pos++;
    }
    va_end(argPointer);
    return 0;
}