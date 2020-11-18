#pragma once

#include <stdint.h>
static inline uint8_t
inb(uint16_t port)
{
    uint8_t data;

    asm volatile("in %1,%0" : "=a" (data) : "d" (port));
    return data;
}

static inline void
insl(int port, void *addr, int cnt)
{
    asm volatile("cld; rep insl" :
    "=D" (addr), "=c" (cnt) :
    "d" (port), "0" (addr), "1" (cnt) :
    "memory", "cc");
}

static inline void
outb(uint16_t port, uint8_t data)
{
    asm volatile("out %0,%1" : : "a" (data), "d" (port));
}

static inline void
outw(uint16_t port, uint16_t data)
{
    asm volatile("out %0,%1" : : "a" (data), "d" (port));
}

static inline void
outsl(int port, const void *addr, int cnt)
{
    asm volatile("cld; rep outsl" :
    "=S" (addr), "=c" (cnt) :
    "d" (port), "0" (addr), "1" (cnt) :
    "cc");
}
