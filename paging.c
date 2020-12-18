#include "paging.h"
#include "vga.h"
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
    // 0x0...8Mb identity mapped
    // KERNEL_HIGH...KERNEL_HIGH+4Mb mapped to 0x0...4Mb
    //

    early_pgdir[0] = PT_PRESENT | PT_WRITEABLE | PT_PAGE_SIZE;
    early_pgdir[PGDIR_IDX(KERNEL_HIGH)] = PT_PRESENT | PT_WRITEABLE | PT_PAGE_SIZE;
    early_pgdir[1] = LARGE_PAGE_SIZE | PT_PRESENT | PT_WRITEABLE | PT_PAGE_SIZE;
}

static void* start_free;
static void* finish_free;
static size_t kalloc_start;
static size_t kalloc_finish;
//0x0...4Mb mapped to 0xFEC00000...0XFFFFFFFF
//KERNEL_HIGH...KERNEL_HIGH + RAM_SIZE mapped to 0x0...RAM_SIZE
//

void identity_map(void* addr, size_t size, int flags) {
    addr = (void*) ROUNDDOWN_LARGE((uint32_t) addr);
    size = ROUNDUP_LARGE(size);
    while (size > 0) {
        kernel_pgdir[PGDIR_IDX((uint32_t)addr)] = ((uint32_t)addr) | flags | PT_PRESENT | PT_PAGE_SIZE;
        size -= LARGE_PAGE_SIZE;
    }
}

void map_high(void* addr, size_t size, int flags) {
    addr = (void*) ROUNDDOWN_LARGE((uint32_t) addr);
    size = ROUNDUP_LARGE(size);
    while (size > 0) {
        kernel_pgdir[PGDIR_IDX((uint32_t)addr + KERNEL_HIGH)] = ((uint32_t)addr) | flags | PT_PRESENT | PT_PAGE_SIZE;
        size -= LARGE_PAGE_SIZE;
    }
}

void reset_ptr(void* ptr) {
    asm volatile ( "invlpg (%0)" : : "b"(ptr) : "memory" );
    asm volatile ("nop");
}
void setup_paging() {

    //terminal_reset();
    struct memory_borders mbd = get_memory_borders();
    kernel_pgdir = (void*)ROUNDUP_LARGE(mbd.start_free);

    start_free = (void*)kernel_pgdir + LARGE_PAGE_SIZE;
    finish_free = (void*)ROUNDDOWN_LARGE(mbd.end_free);

    for (int i = 0; i < 1024; ++i) {
        kernel_pgdir[i] = 0;
    }

    identity_map(0, LARGE_PAGE_SIZE, PT_WRITEABLE);
    map_high(0, LARGE_PAGE_SIZE, PT_WRITEABLE);
    map_high(kernel_pgdir, LARGE_PAGE_SIZE, PT_WRITEABLE);

    identity_map(finish_free, LARGE_PAGE_SIZE, PT_WRITEABLE);

    load_cr3((void*)kernel_pgdir);
    kernel_pgdir = phys2virt(kernel_pgdir);

    // setup phys_kalloc
    kalloc_head.freelist_head = NULL;

    size_t range = 0;
    while (!(kernel_pgdir[range + 1] & PT_PRESENT)) {
        range++;
    }
    size_t pgdir_idx = 1;
    for (void* frame = start_free; frame < finish_free; frame += LARGE_PAGE_SIZE) {

        if (pgdir_idx == range + 1) {
            load_cr3((void*)virt2phys(kernel_pgdir));
            pgdir_idx = 1;
        }
        kernel_pgdir[pgdir_idx] = ((uint32_t)frame) | PT_PRESENT | PT_WRITEABLE | PT_PAGE_SIZE;
        struct fl_entry* frame_ptr = (struct fl_entry*)(pgdir_idx * LARGE_PAGE_SIZE);
        //reset_ptr(frame_ptr);

        frame_ptr->next = kalloc_head.freelist_head;
        kalloc_head.freelist_head = (struct fl_entry*)(frame);
        ++pgdir_idx;
    }
    for (int i = 1; i < range + 1; ++i) {
        kernel_pgdir[i] = 0;
    }
    load_cr3((void*)virt2phys(kernel_pgdir));
}

//return free physical address
void* phys_kalloc(size_t idx, int flags) {
    // TODO: spinlock needed here.

    if (!kalloc_head.freelist_head) {
        return NULL;
    }

    void* ptr = kalloc_head.freelist_head;
    kernel_pgdir[idx] = ((uint32_t)ptr) | flags | PT_PRESENT | PT_PAGE_SIZE;

    reset_ptr((struct fl_entry*)((uint32_t)(idx * LARGE_PAGE_SIZE)));
    struct fl_entry* head_ptr = (struct fl_entry*)((uint32_t)(idx * LARGE_PAGE_SIZE));
    printf(".%x", head_ptr);
    printf(".\n%x ", ptr);
    kalloc_head.freelist_head = head_ptr->next;
    //printf("-> %x\n", virt2phys(ptr));
    return (void*)ptr;
}

void phys_kfree(size_t idx) {
    //printf("<- %x\n", p);
    struct fl_entry* entry = (struct fl_entry*) (idx * LARGE_PAGE_SIZE);

    entry->next = kalloc_head.freelist_head;
    kalloc_head.freelist_head = (struct fl_entry*)ROUNDDOWN_LARGE(kernel_pgdir[idx]);
    kernel_pgdir[idx] = 0;
}

size_t find_free_range(size_t start, size_t finish, size_t pages_cnt) {
    if (pages_cnt == 0) {
        panic("empty requested alloc!");
    }
    size_t len = 0;
    size_t best_len = 0;
    size_t best_start = 0;

    for (size_t pos = start; pos < finish; ++pos) {
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
            best_start = finish - len;
        }
    }

    if (best_len > 0) {
        //printf("\n%u*\n", kernel_pgdir[best_start]);
        return best_start;
    }
    return 0;
}

//flags: PT_USER (do not set if kernel space), PT_WRITEABLE
void* kalloc(size_t size, int flags) {
    flags &= (PT_WRITEABLE | PT_USER);
    size_t page_cnt = ROUNDUP_LARGE(size);
    page_cnt /= LARGE_PAGE_SIZE;

    size_t pagedir_idx;
    if (flags & PT_USER) {
        pagedir_idx = find_free_range(0, KERNEL_HIGH/LARGE_PAGE_SIZE, page_cnt);
    } else {
        pagedir_idx = find_free_range(KERNEL_HIGH/LARGE_PAGE_SIZE, 1024, page_cnt);
    }

    if (pagedir_idx == 0) {
        return NULL;
    }

    for (size_t i = 0; i < page_cnt; ++i) {
        void* phys_ptr = phys_kalloc(pagedir_idx + i, flags);
        //printf("\n%x?\n", phys_ptr);
        if (phys_ptr == NULL) {
            for (size_t j = 0; j < i; ++j) {
                phys_kfree(pagedir_idx + j);
            }
            return NULL;
        }
        //else {
        //    kernel_pgdir[pagedir_idx + i] = (uint32_t)phys_ptr | PT_PRESENT | PT_PAGE_SIZE | PT_WRITEABLE;
        //}
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
        phys_kfree(i);
    }
}

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
