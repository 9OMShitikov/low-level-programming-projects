#pragma once

struct iframe {
    int ip;
    int cs;
    int flags;
    int sp;
    int ss;
};

void init_idt();
void set_irq0_isr();
void unset_irq0_isr();