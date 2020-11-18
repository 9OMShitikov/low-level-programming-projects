#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "idt.h"
#include "gdt.h"
#include "acpi.h"
#include "apic.h"
#include "vga.h"
#include "panic.h"
#include "common.h"

extern void jump_userspace();

void kernel_main(void) {
    init_gdt();
    init_idt();

	terminal_initialize();
    terminal_writestring_color("HeLL OS is loaded.\n", vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    struct acpi_sdt* rsdt = acpi_find_rsdt();
    if (!rsdt) {
        panic("RSDT not found!");
    }

    apic_init(rsdt);

    terminal_change_color(vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_RED));
    asm ("sti");
    //jump_userspace();
}
