#include <io/tty.h>

void initialize()
{
    tty_initialize();
    tty_putstring("Hello ");
    tty_set_color(TTY_COLOR_LIGHT_GREEN, TTY_COLOR_BLACK);
    tty_putstring("world!");
}