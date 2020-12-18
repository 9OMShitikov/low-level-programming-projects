#include "apic.h"
#include "vga.h"
#include "ports.h"

static uint32_t counter = 0;
static uint64_t tmr_delay = 0;

__attribute__ ((interrupt)) void IRQ0_isr(struct iframe* frame) {
    (void) frame;
    __atomic_fetch_add(&counter, 1, __ATOMIC_RELAXED);
    apic_eoi();
}

//setup timer on mcs nanoseconds pause
void PIC_setup_timer(uint32_t ns) {
    uint64_t delay = ns;
    tmr_delay = ns;
    uint64_t divider = 1000000000, default_delay = 50000000,
             min_curr = 1193182;
    //check mcs value
    if (18 * delay > divider || divider > (min_curr - 1) * delay) {
        delay = default_delay;
    }
    uint64_t reload = delay * 1193182;
    reload /= divider;
    outb (0x43, 0b00110100);
    outb(0x40, ((uint8_t*) &reload)[0]);
    outb(0x40, ((uint8_t*) &reload)[1]);
}

//time in ns
void PIC_sleep(uint64_t time) {
    __atomic_store_n(&counter, 0, __ATOMIC_RELAXED);
    uint64_t loaded_counter = 0;
    while (time > loaded_counter * tmr_delay) {
        loaded_counter = __atomic_load_n(&counter, __ATOMIC_RELAXED);
    }
}