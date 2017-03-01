#pragma once

#define PAGE_SIZE 4096

void frame_print_info();
void initialize_from_multiboot(void *multiboot_memory_map_tag);