#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "idt.h"
#include "gdt.h"
#include "acpi.h"
#include "apic.h"
#include "pic.h"
#include "vga.h"
#include "panic.h"
#include "paging.h"
#include "common.h"
#include "multiboot_info.h"
#include "irq.h"

void kernel_main() {
    init_gdt();
    init_idt();

	terminal_initialize();
	printf("Memory areas:\n");
	terminal_change_color(vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
    print_memory_areas();
    terminal_change_color(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));

    setup_paging();

    terminal_writestring_color("HeLL OS is loaded.\n", vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    struct acpi_sdt* rsdt = acpi_find_rsdt();
    if (!rsdt) {
        panic("RSDT not found!");
    }
    apic_init(rsdt);
    //

    enable_irq();

    //*(uint32_t*)0xdeadbeef = 0;
    for (;;) {
        asm volatile ("hlt");
    }
    //jump_userspace();
}
