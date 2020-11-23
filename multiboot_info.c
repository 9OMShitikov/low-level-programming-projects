#include "multiboot_info.h"
#include "vga.h"
multiboot_info_t* mbi;

void init_mbi(multiboot_info_t* mbi_ptr) {
    mbi = mbi_ptr;
};

void print_memory_areas() {
    multiboot_memory_map_t *mmap;

    printf ("mmap_addr = 0x%x, mmap_length = 0x%x\n",
            (unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
    for (mmap = (multiboot_memory_map_t *) mbi->mmap_addr;
         (unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
         mmap = (multiboot_memory_map_t *) ((unsigned long) mmap
                                            + mmap->size + sizeof (mmap->size)))
        printf ("mmap entry: base_addr = 0x%x%x, length = 0x%x%x, type = 0x%x\n",
                (unsigned) (mmap->addr >> 32),
                (unsigned) (mmap->addr & 0xffffffff),
                (unsigned) (mmap->len >> 32),
                (unsigned) (mmap->len & 0xffffffff),
                (unsigned) mmap->type);
}