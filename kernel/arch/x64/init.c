#include <io/tty.h>
#include <printk.h>
#include <mm/frame.h>
#include <mm/memory_map.h>
#include <multiboot.h>
#include <mm/pages.h>
#include <irq.h>

void initialize(void *multiboot_information)
{
    tty_initialize();
    tty_set_color(TTY_COLOR_LIGHT_GREEN, TTY_COLOR_BLACK);
    parse_multiboot(multiboot_information);
    initialize_irq();
    printk("SYSTEM BOOTED\n");
}
