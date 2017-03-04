#include <io/tty.h>
#include <printk.h>
#include <mm/frame.h>
#include <mm/memory_map.h>
#include <multiboot.h>

void initialize(void *multiboot_information)
{
    tty_initialize();
    tty_set_color(TTY_COLOR_LIGHT_GREEN, TTY_COLOR_BLACK);
    printk("KERNEL_VMA=%lx\n", KERNEL_VMA);
    printk("KERNEL_PHYS=%lx\n", KERNEL_PHYS);
    printk("KERNEL_END=%lx\n", KERNEL_END);
    printk("Multiboot information physical address: %lx\n", multiboot_information);
    printk("Multiboot information logical address: %lx\n", multiboot_information + (uint64_t)KERNEL_PHYS);
    printk("Multiboot information logical address 2: %lx\n", multiboot_information + (uint64_t)KERNEL_VMA);

    parse_multiboot(multiboot_information);
    frame_print_info();
}
