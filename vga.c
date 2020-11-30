#include "vga.h"
#include "paging.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

static size_t terminal_row;
static size_t terminal_column;
static uint16_t* terminal_buffer;
static uint8_t default_color;

void terminal_initialize(void) {
	terminal_row = 0;
	terminal_column = 0;
	default_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = phys2virt((uint16_t*) 0xB8000);
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', default_color);
		}
	}
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar_color(char c, uint8_t color) {
    switch (c) {
    case '\n':
        terminal_row++;
        terminal_column = 0;
        if (terminal_row == VGA_HEIGHT) {
            terminal_reset();
        }
        break;

    case '\r':
        terminal_column = 0;
        break;

    default:
        terminal_putentryat(c, color, terminal_column, terminal_row);
        terminal_column++;
        if (terminal_column == VGA_WIDTH) {
            terminal_column = 0;
            terminal_row++;
            if (terminal_row == VGA_HEIGHT) {
                terminal_reset();
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
	terminal_write(data, strlen(data), default_color);
}

void terminal_write_default(const char* data, size_t size) {
    terminal_write(data, size, default_color);
}

void terminal_putchar(char c) {
    terminal_putchar_color(c, default_color);
}

void terminal_writestring_color(const char* data, uint8_t color) {
    terminal_write(data, strlen(data), color);
}

void terminal_reset() {
    terminal_row = 0;
    terminal_column = 0;
    default_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', default_color);
        }
    }
}

void terminal_move_cursor(size_t x, size_t y) {
    if (y >= VGA_HEIGHT || x >= VGA_WIDTH) {
        terminal_row = 0;
        terminal_column = 0;
    } else {
        terminal_row = y;
        terminal_column = x;
    }
}

void terminal_change_color(uint8_t color) {
    default_color = color;
}

void terminal_insertchar(char c) {
    terminal_putentryat(c, default_color, terminal_column, terminal_row);
}

void terminal_rowgo(int x) {
    if (x > 0 && x + terminal_column >= VGA_WIDTH) {
            terminal_column = VGA_WIDTH - 1;
    } else if (x < 0 && terminal_column < -x) {
            terminal_column = 0;
    }
    else terminal_column += x;
}

void terminal_backspace() {
    terminal_rowgo(-1);
    terminal_insertchar(' ');
}
