#pragma once
#include <stddef.h>

#define KERNEL_HEAP_SIZE (8 * 1024 * 1024)
#define KERNEL_HEAP_LEAF 64

void initialize_kernel_heap();
void *kalloc(size_t size);
void kfree(void *ptr);
void *kzalloc(size_t size);
