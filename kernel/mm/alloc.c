#include <mm/alloc.h>
#include <mm/buddy.h>
#include <mm/pages.h>
#include <string.h>

static struct allocator heap_allocator;
const uintptr_t kernel_heap = 0xffffffff81000000;

void initialize_kernel_heap()
{
    size_t bitmap_size = allocator_bitmaps_size(KERNEL_HEAP_SIZE, KERNEL_HEAP_LEAF);
    map_range(kernel_heap, bitmap_size, MAP_RW | MAP_IMMEDIATE);
    map_range(kernel_heap + bitmap_size, KERNEL_HEAP_SIZE, MAP_RW);
    allocator_init(&heap_allocator, (void *) (kernel_heap + bitmap_size), KERNEL_HEAP_SIZE,
                   KERNEL_HEAP_LEAF, (void *) kernel_heap);
    allocator_print_status(&heap_allocator);
}

void *kalloc(size_t size)
{
    return allocator_allocate(&heap_allocator, size);
}

void kfree(void *ptr)
{
    allocator_deallocate(&heap_allocator, ptr);
}

void *kzalloc(size_t size)
{
    void *ptr = kalloc(size);
    memset(ptr, 0, size);
    return ptr;
}
