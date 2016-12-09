#pragma once
#include <stdint.h>

#define FRAME_SIZE 4096
#define LOWER_MEMORY_SIZE (1 * 1024 * 1024)
#define FRAME_USED 0
#define FRAME_FREE 1


struct mmap_entry {
    uint32_t size;
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
};


struct frame_management_data {
	int frames;
	int high_mem_first;
	int next_free_frame;
};


extern int mem_next_frame;


void fill_frame_bitmap_from_memmap(void *boot_information);
void mem_initialize_frame_management();
