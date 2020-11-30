#include "paging.h"
#include "common.h"
#include "panic.h"
#include "multiboot_info.h"
#include <stdint.h>

uint32_t early_pgdir[1024] __attribute__((aligned(4096)));
uint32_t* kernel_pgdir = NULL;

struct fl_entry {
    struct fl_entry* next;
};

struct kalloc_head {
    struct fl_entry* freelist_head;
};

struct kalloc_head kalloc_head;

__attribute__((section(".boot.text"))) void setup_paging_early() {
    // 0x0...4Mb identity mapped
    // KERNEL_HIGH...KERNEL_HIGH+4Mb mapped to 0x0...4Mb

    early_pgdir[0] = PT_PRESENT | PT_WRITEABLE | PT_PAGE_SIZE;
    early_pgdir[PGDIR_IDX(KERNEL_HIGH)] = PT_PRESENT | PT_WRITEABLE | PT_PAGE_SIZE;
    early_pgdir[PGDIR_IDX(KERNEL_HIGH) + 1] = LARGE_PAGE_SIZE | PT_PRESENT | PT_WRITEABLE | PT_PAGE_SIZE;
}

static void* start_free;
static void* finish_free;
static size_t kalloc_start;
static size_t kalloc_finish;
//0x0...4Mb mapped to 0xFEC00000...0XFFFFFFFF
//KERNEL_HIGH...KERNEL_HIGH + RAM_SIZE mapped to 0x0...RAM_SIZE
//
void setup_high_paging() {
    struct memory_borders mbd = get_memory_borders();
    kernel_pgdir = (void*)ROUNDUP_LARGE((uint32_t)&KERNEL_END);
    start_free = kernel_pgdir + LARGE_PAGE_SIZE;
    finish_free = phys2virt(ROUNDDOWN_LARGE(mbd.end_free));
    for (int i = 0; i < 1024; ++i) {
        kernel_pgdir[i] = 0;
    }
    kernel_pgdir[0] = 0xFEC00000 | PT_PRESENT | PT_WRITEABLE | PT_PAGE_SIZE;

    for (uint32_t idx = 0; idx * LARGE_PAGE_SIZE < mbd.end; idx++) {
        kernel_pgdir[PGDIR_IDX(KERNEL_HIGH) + idx] = (idx * LARGE_PAGE_SIZE) | PT_PRESENT | PT_WRITEABLE | PT_PAGE_SIZE;
    }

    kalloc_start = 1;
    kalloc_finish = 1 + (finish_free - start_free) / LARGE_PAGE_SIZE;
    load_cr3(virt2phys(kernel_pgdir));
}

//void setup_physical_mapping

void init_phys_kalloc() {
    void* addr = start_free;
    kalloc_head.freelist_head = NULL;
    while (addr < finish_free) {
        struct fl_entry* entry = (struct fl_entry*)addr;
        entry->next = kalloc_head.freelist_head;
        kalloc_head.freelist_head = entry;
        addr += LARGE_PAGE_SIZE;
    }
}

void* phys_kalloc() {
    // TODO: spinlock needed here.

    if (!kalloc_head.freelist_head) {
        return NULL;
    }

    void* ptr = kalloc_head.freelist_head;
    kalloc_head.freelist_head = kalloc_head.freelist_head->next;
    //printf("-> %x\n", virt2phys(ptr));
    return virt2phys(ptr);
}

void phys_kfree(void* p) {
    //printf("<- %x\n", p);
    struct fl_entry* entry = (struct fl_entry*)phys2virt(p);
    entry->next = kalloc_head.freelist_head;
    kalloc_head.freelist_head = entry;
}

size_t find_free_range(size_t pages_cnt) {
    if (pages_cnt == 0) {
        panic("empty requested alloc!");
    }
    size_t len = 0;
    size_t best_len = 0;
    size_t best_start = 0;

    kalloc_finish = kalloc_start + 10;
    for (size_t pos = kalloc_start; pos < kalloc_finish; ++pos) {
        //printf("%u,", kernel_pgdir[pos]);
        if (kernel_pgdir[pos] & PT_PRESENT) {
            if (len >= pages_cnt) {
                if (len < best_len || best_len == 0) {
                    best_len = len;
                    best_start = pos - len;
                }
            }
            len = 0;
        }
        else {
            len++;
        }
    }
    //printf("\n");
    if (len >= pages_cnt) {
        if (len < best_len || best_len == 0) {
            best_len = len;
            best_start = kalloc_finish - len;
        }
    }

    if (best_len > 0) {
        return best_start;
    }
    return 0;
}

void* kalloc(size_t size) {
    size_t page_cnt = ROUNDUP_LARGE(size);
    page_cnt /= LARGE_PAGE_SIZE;

    size_t pagedir_idx = find_free_range(page_cnt);

    if (pagedir_idx == 0) {
        return NULL;
    }

    for (size_t i = 0; i < page_cnt; ++i) {
        void* phys_ptr = phys_kalloc();
        if (phys_ptr == NULL) {
            for (size_t j = 0; j < i; ++j) {
                void* page_ptr = ROUNDDOWN_LARGE(kernel_pgdir[pagedir_idx + j]);
                kernel_pgdir[pagedir_idx + j] = 0;
                phys_kfree(page_ptr);
            }
            return NULL;
        }
        else {
            kernel_pgdir[pagedir_idx + i] = (uint32_t)phys_ptr | PT_PRESENT | PT_PAGE_SIZE | PT_WRITEABLE;
        }
    }
    pagedir_idx *= LARGE_PAGE_SIZE;
    return (void*) (pagedir_idx);
}

void kfree(void* ptr, size_t size) {
    size_t page_cnt = ROUNDUP_LARGE(size);
    page_cnt /= LARGE_PAGE_SIZE;
    //printf("$#%d\n", page_cnt);

    size_t start_idx = ((size_t)(ROUNDDOWN_LARGE((uint32_t)ptr))) / LARGE_PAGE_SIZE;
    //printf("$@%d\n", start_idx);
    for (size_t i = start_idx; i < page_cnt + start_idx; ++i) {
        void* addr = (void*) ROUNDDOWN_LARGE(kernel_pgdir[i]);
        phys_kfree(addr);
        kernel_pgdir[i] = 0;
    }
}

//void map_continous(uint32_t* pgdir, void* addr, size_t size, void* phys_addr, int writeable) {
//    addr = (void*)ROUNDDOWN_LARGE((uint32_t)addr);
//    phys_addr = (void*)ROUNDDOWN_LARGE((uint32_t)phys_addr);
//    size = ROUNDUP_LARGE(size);
//
//    while (size > 0) {
//        uint32_t* pte = alloc_page(pgdir, addr);
//        *pte = ((uint32_t)phys_addr) | PT_PRESENT;
//        if (writeable) {
//            *pte |= PT_WRITEABLE;
//        }
//        addr += PAGE_SIZE;
//        phys_addr += PAGE_SIZE;
//        size -= PAGE_SIZE;
//    }
//}

void load_cr3(uint32_t* pgdir) {
    asm volatile (
        "mov %0, %%cr3\n"
        :
        : "r"(pgdir)
        : "memory"
    );
}


uint32_t read_cr2() {
    uint32_t val = 0;
    asm volatile (
        "mov %%cr2, %0\n"
        : "=r"(val)
    );
    return val;
}

__attribute__ ((interrupt)) void pagefault_isr(struct iframe* frame, uint32_t error_code) {
    (void)frame;
    (void)error_code;

    //*(uint32_t*)0xdeadbeef = 0;
    //
    //void* addr = (void*)read_cr2();
    //void* new_page = kalloc();
    //if (!new_page) {
        panic("cannot allocate new page");
    //}
    //map_continous(kernel_pgdir, addr, PAGE_SIZE, virt2phys(new_page), 1);
}
