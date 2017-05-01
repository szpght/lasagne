#include <multiboot.h>
#include <mm/frame.h>
#include <mm/memory_map.h>
#include <printk.h>

void parse_multiboot(void *mb)
{
    mb += (uint64_t) KERNEL_VMA;
    uint32_t size = ((struct mb_header*) mb)->total_size;

    printk("Mutiboot information size: %u\n", size);
    mb += 8;
    size -= 8;

    while (size > 0) {
        struct mb_tag *mbt = mb;
        uint32_t tag_size = mbt->size;
        if (tag_size % 8) {
            tag_size += 8 - tag_size % 8;
        }
        //printk("Tag hit, type %u, size %u, addr %lx\n", mbt->type, mbt->size, mbt);
        switch (mbt->type) {
        case MB_MEMORY_MAP:
            parse_memory_map(mbt);
            break;
        }
        mb += tag_size;
        size -= tag_size;
    }
}

void parse_memory_map(void *mb)
{
    fill_memory_map(mb);
    mem_map_sanitize();
}

void fill_memory_map(void *mb)
{
    struct mb_memmap *mm = mb;

    int entries = (mm->size - sizeof(*mm)) / mm->entry_size;
    struct mb_memmap_entry *entry = mb + sizeof(*mm);
    for (int i = 0; i < entries; ++i) {
        if (entry->type == MB_RAM_AVAILABLE || entry->type == MB_RAM_ACPI) {
            printk("Adding usable memory of size %ld at %lx to map\n", entry->length, entry->base_addr);
            mem_map_add_area(entry->base_addr, entry->base_addr + entry->length);
        }
        ++entry;
    }
}
