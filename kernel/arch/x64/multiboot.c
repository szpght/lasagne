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

    for (int i = 0; i < size; ++i) {
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
        printk("Tag hit, type %u, size %u, addr %lx\n", mbt->type, mbt->size, mbt);
        switch (mbt->type) {
        case MB_MEMORY_MAP:
            initialize_memory(mbt);
            break;
        }
        mb += tag_size;
        size -= tag_size;
    }
    printk("End of multiboot information\n");
}

void initialize_memory(void *mb)
{
    size_t size = (size_t) physical_end(mb);
    frame_init(size);
    mark_free(mb);

}

void *physical_end(void *mb)
{
    void *end = 0;
    struct mb_memmap *mm = mb;

    int entries = (mm->size - sizeof(*mm)) / mm->entry_size;    
    struct mb_memmap_entry *entry = mb + sizeof(*mm);
    for (int i = 0; i < entries; ++i) {
        void *current = entry->base_addr + entry->length;
        if (current > end) {
            end = current;
        }
        ++entry;
    }
}

void mark_free(void *mb)
{
    struct mb_memmap *mm = mb;

    int entries = (mm->size - sizeof(*mm)) / mm->entry_size;
    struct mb_memmap_entry *entry = mb + sizeof(*mm);
    for (int i = 0; i < entries; ++i) {
        printk ("memory area: base=%lx, size=%lx, type=%d\n",
            entry->base_addr, entry->length, entry->type);

        if (entry->type == MB_RAM_AVAILABLE || entry->type == MB_RAM_ACPI) {
            add_free_space((void*)entry->base_addr, entry->length);
        }
        ++entry;
    }
}
