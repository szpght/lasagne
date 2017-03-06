#include <mm/alloc.h>
#include <mm/frame.h>
#include <mm/memory_map.h>

void *kalloc(size_t size)
{
    size_t alloc_size = PAGE_SIZE;
    while (alloc_size < size) {
        alloc_size <<= 1;
    }
    return allocator_allocate(&frame_alloc, alloc_size) - (uint64_t)KERNEL_PHYS;
}

void kfree(void *ptr)
{
    allocator_deallocate(&frame_alloc, ptr + (uint64_t)KERNEL_PHYS);
}