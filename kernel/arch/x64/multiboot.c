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
            initialize_from_multiboot(mbt);
            break;
        }
        mb += tag_size;
        size -= tag_size;
    }
    printk("End of multiboot information\n");
}
