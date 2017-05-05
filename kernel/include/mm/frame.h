#pragma once
#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096

struct allocator_node {
    uint64_t next;
    uint64_t frames[511];
};

struct allocator {
    uint64_t begin;
    uint64_t end;
    size_t size;
    size_t capacity;
    size_t free;
    struct allocator_node *current;
    int count;
    int current_map_position;
    struct allocator_node *current_address;
    uint64_t *current_pte;
};

extern struct allocator default_frame_allocator;

void alloc_init(struct allocator *alloc, uint64_t begin, uint64_t end,
                void *current_address, uint64_t *current_pte);
uint64_t alloc_frame(struct allocator *alloc);
void dealloc_frame(struct allocator *alloc, uint64_t frame);
void initialize_frame_allocation();
void print_frame_allocation_info();
uint64_t get_frame();
void free_frame(uint64_t frame);
