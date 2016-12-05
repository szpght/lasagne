#include <kernel/tty.h>
#include <stddef.h>
#include "memory_sections.h"

/* this is not a part of tty interface */
void tty_initialize(void);

static const char kernel_greeting[] = "Hello, World!";
void
kernel_main(/* ... */)
{
    tty_initialize();
    for (char *s = kernel_greeting; *s; s++)
        tty_putchar(*s);
}
