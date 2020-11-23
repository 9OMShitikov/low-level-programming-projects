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

__attribute__ ((interrupt)) void keyboard_isr(struct iframe* frame) {
    (void)frame;

    spin_lock(&lock);

    keyboard_interact();

    spin_unlock(&lock);

    apic_eoi();
}

uint32_t ticks = 0;
__attribute__ ((interrupt)) void timer_isr(struct iframe* frame) {
    (void)frame;

    __atomic_fetch_add(&ticks, 1, __ATOMIC_RELAXED);
    //spin_lock(&lock);
    //terminal_writestring_color(".", vga_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK));
    //spin_unlock(&lock);

    apic_eoi();
}

uint32_t get_ticks() {
    return __atomic_load_n(&ticks, __ATOMIC_RELAXED);
}

void ticks_set(uint32_t value) {
    __atomic_store_n(&ticks, value, __ATOMIC_RELAXED);
}

void ticks_reset() {
    __atomic_store_n(&ticks, 0, __ATOMIC_RELAXED);
}