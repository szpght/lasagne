// physical frame allocator
#include <mm/buddy.h>
#include <mm/frame.h>
#include <mm/memory_map.h>
#include <multiboot.h>
#include <stddef.h>
#include <printk.h>


static void add_free_space(void *base, size_t size);


struct allocator frame_alloc;
#define alloc (&frame_alloc)


void frame_print_info()
{
    printk("Available RAM: %lu KiB", frame_alloc.free_size / 1024);
}


void initialize_from_multiboot(void *mb)
{
    allocator_init(alloc, KERNEL_VMA, 1024 * 1024 * 1024, PAGE_SIZE, KERNEL_END);

    struct mb_memmap *mm = mb;

    int entries = (mm->size - sizeof(*mm)) / mm->entry_size;
    struct mb_memmap_entry *entry = mb + sizeof(*mm);
    for (int i = 0; i < entries; ++i) {
        if (entry->type == MB_RAM_AVAILABLE || entry->type == MB_RAM_ACPI) {
            add_free_space((void*)entry->base_addr, entry->length);
        }
        ++entry;
    }
}

static void add_free_space(void *base, size_t size)
{
    printk ("free space: base=%lx, size=%lx\n", base, size);
    void *start = base + (uint64_t)KERNEL_VMA;
    void *end = start + size;

    // if area starts in kernel memory, move start after kernel memory
    if (start >= KERNEL_VMA && start < KERNEL_END) {
        start = KERNEL_END;
    }

    // if area ends in kernel memory, there's no point mapping it
    if (end >= KERNEL_VMA && end < KERNEL_END) {
        printk("returning from add_free_space\n");
        return;

    }

    allocator_init_free(alloc, start, end);
}