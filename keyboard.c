#include <stdint.h>
#include "keyboard.h"
#include "common.h"

#include "idt.h"
#include "ports.h"
#include "keyboard_config.h"
#include "vga.h"

static uint32_t E0 = 0;
static const int wrong_symbol = -300;

static struct keyboard_state state;

struct keyboard_state get_keyboard_state() {
    return state;
}

int32_t keyboard_get_char() {
    uint8_t st = inb(KBSTATP);
    if((st & KBS_DIB) == 0)
        return wrong_symbol;
    uint8_t data = inb(KBDATAP);
    //printf ("%x\n", data);
    if (data == 0xE0) {
        E0 = 1;
        return wrong_symbol;
    } else {
        int ret_mul = 1;
        if (data & 0x80) {
            ret_mul = -1;
            data &= ~0x80;
        }
        if (E0) {
            E0 = 0;
            return ret_mul * E0_table[data];
        }
        if (state.shift_pressed || state.caps_on) {
            return ret_mul * shift_table[data];
        }
        return ret_mul * default_table[data];
    }
}

void keyboard_interact() {
    int symbol = keyboard_get_char();
    if (symbol != wrong_symbol) {
        if (symbol > 0) {
            switch (symbol) {
                case (KEY_UP): {
                }
                    break;
                case (KEY_DN): {
                }
                    break;
                case (KEY_LF): {
                    terminal_rowgo(-1);
                    //terminal_insertchar('0');
                }
                    break;
                case (KEY_RT): {
                    terminal_rowgo(1);
                    //terminal_insertchar('1');
                }
                    break;
                case (SHIFT): {
                    state.shift_pressed = 1;
                }
                break;
                case (CTRL): {
                    state.ctrl_pressed = 1;
                }
                break;
                case (NUMLOCK): {
                    state.numlock_pressed = 1;
                }
                break;
                case (ALT): {
                    state.alt_pressed = 1;
                }
                break;
                case (CAPSLOCK): {
                    state.capslock_pressed = 1;
                }
                break;
                case (BACKSPACE): {
                    terminal_backspace();
                }
                break;
                default: {
                    terminal_putchar(symbol);
                }
            }
        }
        else {

            switch (-symbol) {
                case (KEY_UP): {

                }
                    break;
                case (KEY_DN): {

                }
                    break;
                case (KEY_LF): {

                }
                    break;
                case (KEY_RT): {

                }
                    break;
                case (SHIFT): {
                    state.shift_pressed = 0;
                }
                    break;
                case (CTRL): {
                    state.ctrl_pressed = 0;
                }
                    break;
                case (NUMLOCK): {
                    state.numlock_pressed = 0;
                    state.num_on = ~state.num_on;
                }
                    break;
                case (ALT): {
                    state.alt_pressed = 0;
                }
                    break;
                case (CAPSLOCK): {
                    state.capslock_pressed = 0;
                    state.caps_on = ~state.caps_on;
                }
                break;
                case (BACKSPACE): {

                }
                    break;
                default: {
                }
            }
        }
    }
}
