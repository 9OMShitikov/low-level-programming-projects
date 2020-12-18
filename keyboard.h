#pragma once

#include <stdint.h>
#include "idt.h"

struct keyboard_state {
    uint32_t shift_pressed;
    uint32_t capslock_pressed;
    uint32_t alt_pressed;
    uint32_t ctrl_pressed;
    uint32_t numlock_pressed;
    uint32_t caps_on;
    uint32_t num_on;
};

struct keyboard_state get_keyboard_state();
//Keyboard support states of shift, ctl, alt, caps_lck and num_lck, uppercase letters on shift and caps lock
void keyboard_interact();

