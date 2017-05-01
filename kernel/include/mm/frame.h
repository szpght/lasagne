#pragma once
#include <stddef.h>
#include <mm/buddy.h>

#define PAGE_SIZE 4096

extern struct allocator frame_alloc;

void initialize_memory();
void frame_print_info();
void add_free_space(void *base, size_t size);
void frame_init(size_t size);
void map_physical(size_t size);
void mark_free();
