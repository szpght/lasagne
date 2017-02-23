#pragma once
#include <types.h>

enum tty_color {
    TTY_COLOR_BLACK = 0,
    TTY_COLOR_BLUE = 1,
    TTY_COLOR_GREEN = 2,
    TTY_COLOR_CYAN = 3,
    TTY_COLOR_RED = 4,
    TTY_COLOR_MAGENTA = 5,
    TTY_COLOR_BROWN = 6,
    TTY_COLOR_LIGHT_GREY = 7,
    TTY_COLOR_DARK_GREY = 8,
    TTY_COLOR_LIGHT_BLUE = 9,
    TTY_COLOR_LIGHT_GREEN = 10,
    TTY_COLOR_LIGHT_CYAN = 11,
    TTY_COLOR_LIGHT_RED = 12,
    TTY_COLOR_LIGHT_MAGENTA = 13,
    TTY_COLOR_LIGHT_BROWN = 14,
    TTY_COLOR_WHITE = 15,
};


struct tty_style {
    enum tty_color fg;
    enum tty_color bg;
    uint8_t flags;
};

void tty_putchar(uint32_t c);
void tty_initialize();
void tty_set_color(enum tty_color foreground, enum tty_color background);
void tty_putstring(char *string);
