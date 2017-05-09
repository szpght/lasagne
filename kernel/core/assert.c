#include <assert.h>
#include <irq.h>
#include <printk.h>

void halt()
{
    disable_irq();
    printk("System halted");
    for (;;) {
        __asm__ volatile("hlt");
    }
}

void _assert_backend(char *exp, char *file, int line)
{
    printk("%s:%d: assertion %s failed\n", file, line, exp);
    halt();
}

void panic(char *msg)
{
    printk("Kernel panic\n%s\n", msg);
    halt();
}