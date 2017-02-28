#include <io/tty.h>
#include <printk.h>

void initialize(void *multiboot_information)
{
    tty_initialize();
    tty_set_color(TTY_COLOR_LIGHT_GREEN, TTY_COLOR_BLACK);
    printk("HACKING IN PROGRESS %lx\n", initialize);
}