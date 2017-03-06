// physical frame allocator
#include <mm/buddy.h>
#include <mm/frame.h>
#include <mm/memory_map.h>
#include <mm/pages.h>
#include <multiboot.h>
#include <stddef.h>
#include <printk.h>


struct allocator frame_alloc;
#define alloc (&frame_alloc)


void frame_print_info()
{
    allocator_print_status(alloc);
}

void frame_init(size_t size)
{
    // if you have < 2 MiB of RAM, you have bad luck
    size_t alloc_size = 2 * 1024 * 1024;

    while (alloc_size < size) {
        alloc_size *= 2;
    }
    map_physical(size);
    allocator_init(alloc, KERNEL_PHYS, alloc_size, PAGE_SIZE, KERNEL_END_PHYS);
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

void map_physical(size_t size)
{
    // setup pdpt for physical memory
    uint64_t *pdpt = KERNEL_END;
    KERNEL_END += 4096;

    // write pdpt to pml4t
    PML4T_P[256] = (void*)pdpt - (uint64_t)KERNEL_VMA;
    PML4T[256].p = 1;
    PML4T[256].rw = 1;

    // clean pdpt
    for (int i = 0; i < 512; ++i) {
        pdpt[i] = 0;
    }

    // fill as many pd's as needed
    uint64_t *pd = KERNEL_END;
    int pages = 0;
    size_t filled_size = 0;
    while (filled_size < size) {
        *pd = filled_size | PG_PRESENT | PG_RW | PG_HUGE;
        ++pd;
        ++pages;
        filled_size += 2 * 1024 * 1024;
    }

    // fill the rest of pd
    int rest = 512 - pages % 512;
    for (int i = 0; i < rest; ++i) {
        *pd = 0;
        ++pd;
    }

    int number_of_pds = ((void *)pd - KERNEL_END) / 4096;
    pd = KERNEL_END - (uint64_t)KERNEL_VMA;

    // write pds to pdpt
    for (int i = 0; i < number_of_pds; ++i) {
        pdpt[i] = (uint64_t)pd | PG_PRESENT | PG_RW;
        pd += 512;
    }

    KERNEL_END = (uint64_t)pd + KERNEL_VMA;
    reload_paging();
}
