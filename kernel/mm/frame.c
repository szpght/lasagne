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
    printk ("add_free_space called, base=%lx, size=%u\n", base, size);
    void *start = base + (uint64_t)KERNEL_VMA;
    void *end = start + size;

    for (; start < end; start += PAGE_SIZE) {
        if (start >= KERNEL_VMA && start < KERNEL_END) continue;

        allocator_init_free(alloc, start, start + PAGE_SIZE);
    }
}