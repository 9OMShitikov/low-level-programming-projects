#include "multiboot_info.h"
#include "vga.h"
#include "paging.h"
#include "panic.h"
multiboot_info_t* mbi;
uint32_t magic;

void init_mbi(multiboot_info_t* mbi_ptr) {
    mbi = mbi_ptr;
};

struct memory_borders get_memory_borders() {
    struct memory_borders result;
    result.start_free = 0x100000;

    multiboot_memory_map_t *mmap;
    for (mmap = (multiboot_memory_map_t *) mbi->mmap_addr;
         (unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
         mmap = (multiboot_memory_map_t *) ((unsigned long) mmap
                                            + mmap->size + sizeof (mmap->size))) {

        if ((unsigned) (mmap->addr & 0xffffffff) == result.start_free) {
            result.end_free = (unsigned)((mmap->addr + mmap->len) & 0xffffffff);
            mmap = (multiboot_memory_map_t *) ((unsigned long) mmap
                                               + mmap->size + sizeof (mmap->size));
            if ((unsigned long) mmap >= mbi->mmap_addr + mbi->mmap_length) {
                panic("mmap entries ended too soon!");
            }
            result.end = (unsigned)((mmap->addr + mmap->len) & 0xffffffff);
            return result;
        }
    }
    panic("mmap start address not found!");
}

void print_memory_areas() {
    multiboot_memory_map_t *mmap;
    printf ("mmap_addr = 0x%x, mmap_length = 0x%x\n",
            (unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);

    for (mmap = (multiboot_memory_map_t *) mbi->mmap_addr;
         (unsigned long) mmap < (uint32_t)mbi->mmap_addr + mbi->mmap_length;
         mmap = (multiboot_memory_map_t *) ((unsigned long) mmap
                                            + mmap->size + sizeof (mmap->size))) {

        printf("mmap entry: base_addr = 0x%x%x, length = 0x%x%x, type = 0x%x\n",
               (unsigned) (mmap->addr >> 32),
               (unsigned) (mmap->addr & 0xffffffff),
               (unsigned) (mmap->len >> 32),
               (unsigned) (mmap->len & 0xffffffff),
               (unsigned) mmap->type);
    }
}