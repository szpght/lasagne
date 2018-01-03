#include <mm/alloc.h>
#include <mm/buddy.h>
#include <mm/memory_map.h>
#include <mm/pages.h>
#include <string.h>
#include <util/math.h>

static struct allocator heap_allocator;
const uintptr_t kernel_heap = 0xffffffff81000000;

__init void initialize_kernel_heap()
{
    size_t bitmap_size = allocator_bitmaps_size(KERNEL_HEAP_SIZE, KERNEL_HEAP_LEAF);
    map_range(kernel_heap, bitmap_size, MAP_RW | MAP_IMMEDIATE);
    map_range(kernel_heap + bitmap_size, KERNEL_HEAP_SIZE, MAP_RW);
    allocator_init(&heap_allocator, (void *) (kernel_heap + bitmap_size), KERNEL_HEAP_SIZE,
                   KERNEL_HEAP_LEAF, (void *) kernel_heap);
    print_heap_status();
}

void *kalloc(size_t size)
{
    return allocator_allocate(&heap_allocator, size);
}

void *krealloc(void* ptr, size_t size)
{
    size_t current_size = allocator_allocation_size(&heap_allocator, ptr);
    size_t new_size = ceil_to_power_of_two(size);

    if (new_size == current_size) {
        return ptr;
    }

    void *new_ptr = kalloc(new_size);
    memcpy(new_ptr, ptr, current_size);
    allocator_deallocate_fast(&heap_allocator, ptr, current_size);
    return new_ptr;
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

void print_heap_status()
{
    allocator_print_status(&heap_allocator);
}
