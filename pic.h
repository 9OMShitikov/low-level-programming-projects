#pragma once

//setup timer on mcs nanoseconds pause
void PIC_setup_timer(uint32_t ns);

//time in ns
void PIC_sleep(uint64_t time);
