#include <multiboot.h>
#include <mm/frame.h>
#include <mm/memory_map.h>
#include <printk.h>

void parse_multiboot(void *mb)
{
    // copy multiboot information to NULL
    uint8_t *dest = KERNEL_VMA;
    uint8_t *src = mb;
    uint32_t size = ((struct mb_header*) mb)->total_size;

    for (uint32_t i = 0; i < size; ++i) {
        dest[i] = src[i];
    }
    mb = KERNEL_VMA;

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
            initialize_memory(mbt);
            break;
        }
        mb += tag_size;
        size -= tag_size;
    }
    //printk("End of multiboot information\n");
}

void initialize_memory(void *mb)
{
    size_t size = (size_t) physical_end(mb);
    printk("Mapping physical memory to kernel address space... ");
    frame_init(size);
    printk("mapped\n");
    mark_free(mb);
}

void *physical_end(void *mb)
{
    void *end = 0;
    struct mb_memmap *mm = mb;

    int entries = (mm->size - sizeof(*mm)) / mm->entry_size;    
    struct mb_memmap_entry *entry = mb + sizeof(*mm);
    for (int i = 0; i < entries; ++i) {
        void *current = (void*)entry->base_addr + entry->length;
        printk ("memory area: base=%lx, size=%ld KiB, type=%d\n",
            entry->base_addr, entry->length / 1024, entry->type);
        if (current > end) {
            end = current;
        }
        ++entry;
    }
    return end;
}

void mark_free(void *mb)
{
    struct mb_memmap *mm = mb;

    int entries = (mm->size - sizeof(*mm)) / mm->entry_size;
    struct mb_memmap_entry *entry = mb + sizeof(*mm);
    for (int i = 0; i < entries; ++i) {
        if (entry->type == MB_RAM_AVAILABLE || entry->type == MB_RAM_ACPI) {
            printk("Adding usable memory of size %ld at %lx\n", entry->length, entry->base_addr);
            add_free_space((void*)entry->base_addr, entry->length);
        }
        ++entry;
    }
}
