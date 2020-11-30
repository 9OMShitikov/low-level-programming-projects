#pragma once

#include <stddef.h>
#include <stdint.h>
#include "defs.h"

#define PAGE_SIZE 4096
#define LARGE_PAGE_SIZE 4194304

#define PT_PRESENT      (1 << 0)
#define PT_WRITEABLE    (1 << 1)
#define PT_USER         (1 << 2)
#define PT_PAGE_SIZE    (1 << 7)

#define PGDIR_IDX(addr) ((((uint32_t)addr) >> 22) & 1023)
#define PT_IDX(addr) ((((uint32_t)addr) >> 12) & 1023)

#define ROUNDUP(addr) ((void*)(((uint32_t)(addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1)))
#define ROUNDDOWN(addr) ((void*)((uint32_t)(addr) & ~(PAGE_SIZE - 1)))

#define ROUNDUP_LARGE(addr) ((void*)(((uint32_t)(addr) + LARGE_PAGE_SIZE - 1) & ~(LARGE_PAGE_SIZE - 1)))
#define ROUNDDOWN_LARGE(addr) ((void*)((uint32_t)(addr) & ~(LARGE_PAGE_SIZE - 1)))

extern char KERNEL_END[];
extern char KERNEL_START[];

static inline void* virt2phys(void* addr) {
    return (void*)((uint32_t)addr - KERNEL_HIGH);
}

static inline void* phys2virt(void* addr) {
    return (void*)((uint32_t)addr + KERNEL_HIGH);
}

void setup_high_paging();

void init_phys_kalloc() ;

void* phys_kalloc();

size_t find_free_range(size_t pages_cnt);

void* kalloc(size_t size);

void kfree(void* ptr, size_t size);