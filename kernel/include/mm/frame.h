#pragma once
#include <stddef.h>

#define PAGE_SIZE 4096

void frame_print_info();
void initialize_from_multiboot(void *multiboot_memory_map_tag);
void add_free_space(void *base, size_t size);
