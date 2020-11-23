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
#include "common.h"
#include "multiboot_info.h"

extern void jump_userspace();

void kernel_main(multiboot_info_t* addr, uint32_t magic) {
    init_gdt();
    init_idt();
    init_mbi(addr);

	terminal_initialize();
    terminal_writestring_color("HeLL OS is loaded.\n", vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    struct acpi_sdt* rsdt = acpi_find_rsdt();
    if (!rsdt) {
        panic("RSDT not found!");
    }

    apic_init(rsdt);
    print_memory_areas();
    asm ("sti");
    //jump_userspace();
}
