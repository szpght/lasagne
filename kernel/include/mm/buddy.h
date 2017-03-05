#pragma once
#include <stddef.h>
#include <stdint.h>

// 40 levels + 12 bits of page size gives 52 bits - physical address limit
#define ALLOCATOR_MAX_LEVELS 40
#define ALLOCATOR_EMPTY ((void *)-1)

struct allocator_node {
    struct allocator_node *prev;
    struct allocator_node *next;
};


struct allocator {
    void *memory;
    size_t size;
    size_t free_size;
    size_t leaf_size;
    int max_level;
    int bitmap_entries;
    struct allocator_node *free_blocks[ALLOCATOR_MAX_LEVELS];
    size_t free_count[ALLOCATOR_MAX_LEVELS];
    uint8_t *split_bitmap;
    uint8_t *allocation_bitmap;
};

void allocator_print_status(struct allocator *alloc);
void allocator_init(struct allocator *alloc, void *memory, size_t size,
                    size_t leaf_size, void *bitmaps);
void *allocator_allocate(struct allocator *allocator, size_t size);
void allocator_deallocate_fast(struct allocator *allocator, void *address, size_t size);
void allocator_deallocate(struct allocator *allocator, void *block);
size_t allocator_bitmaps_size(size_t memory_size, size_t leaf_size);
int allocator_init_free(struct allocator *alloc, void *begin, void *end);
void allocator_init_free_auto(struct allocator *alloc);
