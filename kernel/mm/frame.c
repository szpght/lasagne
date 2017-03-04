// physical frame allocator
#include <mm/buddy.h>
#include <mm/frame.h>
#include <mm/memory_map.h>
#include <multiboot.h>
#include <stddef.h>
#include <printk.h>


struct allocator frame_alloc;
#define alloc (&frame_alloc)


void frame_print_info()
{
    printk("Available RAM: %lu KiB\n", frame_alloc.free_size / 1024);
}

void frame_init(size_t size)
{
    // if you have < 2 MiB of RAM, you have bad luck
    size_t alloc_size = 2 * 1024 * 1024;

    while (alloc_size < size) {
        alloc_size *= 2;
    }
    allocator_init(alloc, KERNEL_PHYS, alloc_size, PAGE_SIZE, KERNEL_END);
}

void add_free_space(void *base, size_t size)
{
    void *start = base + (uint64_t)KERNEL_PHYS;
    void *end = start + size;

    // if area starts in kernel memory, move start after kernel memory
    if (start >= KERNEL_PHYS && start < KERNEL_END_PHYS) {
        start = KERNEL_END_PHYS;
    }

    // if area ends in kernel memory, there's no point mapping it
    // as there's nothing to map before kernel
    if (end >= KERNEL_PHYS && end < KERNEL_END_PHYS) {
        return;
    }

    allocator_init_free(alloc, start, end);
}
