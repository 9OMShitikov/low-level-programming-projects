#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "idt.h"
#include "apic.h"
#include "vga.h"
#include "panic.h"
#include "spinlock.h"
#include "common.h"
#include "keyboard.h"
static int lock = 0;

__attribute__ ((interrupt)) void syscall_entry(struct iframe* frame) {
    terminal_writestring_color("Syscall!\n", vga_entry_color(VGA_COLOR_RED, VGA_COLOR_BLACK));
    (void)frame;
}

__attribute__ ((interrupt)) void timer_isr(struct iframe* frame) {
    (void)frame;

    spin_lock(&lock);
    //terminal_writestring_color(".", vga_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK));
    spin_unlock(&lock);

    apic_eoi();
}

__attribute__ ((interrupt)) void keyboard_isr(struct iframe* frame) {
    (void)frame;

    spin_lock(&lock);

    keyboard_interact();
    //terminal_writestring_color(".", vga_entry_color(VGA_COLOR_RED, VGA_COLOR_GREEN));
    spin_unlock(&lock);

    apic_eoi();
}