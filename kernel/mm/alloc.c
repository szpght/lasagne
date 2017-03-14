#include <mm/alloc.h>
#include <mm/frame.h>
#include <mm/memory_map.h>
#include <string.h>

void *kalloc(size_t size)
{
    size_t alloc_size = PAGE_SIZE;
    while (alloc_size < size) {
        alloc_size <<= 1;
    }
    void *ptr = allocator_allocate(&frame_alloc, alloc_size);
    return ptr;
}

void kfree(void *ptr)
{
    allocator_deallocate(&frame_alloc, ptr);
}

void *kzalloc(size_t size)
{
	void *ptr = kalloc(size);
	memset(ptr, 0, size);
	return ptr;
}
