#include <mm/memory_map.h>
#include <io/pit.h>
#include <irq.h>
#include <io/ports.h>
#include <printk.h>
#include <task.h>

static void set_counter()
{
    int target_freq_hz = 1000;
    int divisor = 1193182 / target_freq_hz;
    outb(PIT_CH0, divisor);
    outb(PIT_CH0, divisor >> 8);
}

__init void pit_initialize()
{
    printk("hello pit world\n");
    set_irq_handler(0x20, pit_handler, 0);
    // set mode
    outb(PIT_COMMAND, PIT_CH0_SQUARE);

    set_counter();
    pic_flip_irq(0);
}

void pit_handler()
{
    irq_eoi();
    preempt_int();
}
