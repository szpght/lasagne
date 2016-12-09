#include <kernel/tty.h>

static const char kernel_greeting[] = "Hello, World!";

int kernel_main() {
	for (char *s = kernel_greeting; *s; s++)
    tty_putchar(*s);
}