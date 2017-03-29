#include <io/tty.h>
#include <printk.h>
#include <mm/frame.h>
#include <mm/memory_map.h>
#include <multiboot.h>
#include <mm/pages.h>
#include <irq.h>
#include <task.h>

void initialize(void *multiboot_information)
{
    tty_initialize();
    tty_set_color(TTY_COLOR_LIGHT_GREEN, TTY_COLOR_BLACK);
    parse_multiboot(multiboot_information);
    initialize_irq();
    initialize_tasks();
    printk("SYSTEM BOOTED\n");
    int counter = 0;
    while (1) {
        for (int i = 0; i < 50000000; ++i) {
            __asm__ volatile ("nop");
        }
        printk("A %d\n", ++counter);
        __asm__ volatile ("int $48");
    }
}
