#pragma once
#include <stdint.h>


typedef struct mmap_entry {
    uint32_t size;
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
};

void fill_frame_bitmap_from_memmap(void *boot_information);