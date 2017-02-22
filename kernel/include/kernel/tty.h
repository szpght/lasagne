#ifndef KERNEL_TTY_H
#define KERNEL_TTY_H

#include <stdint.h>

/**
 * Standard VGA colors.
 * Terminal drivers SHOULD NOT rely on numerical values of these constants.
 */
enum tty_color {
    TTY_COLOR_NONE,
    TTY_COLOR_BLACK,
    TTY_COLOR_GRAY,
    TTY_COLOR_SILVER,
    TTY_COLOR_WHITE,
    TTY_COLOR_RED,
    TTY_COLOR_GREEN,
    TTY_COLOR_BLUE,
    TTY_COLOR_CYAN,
    TTY_COLOR_MAGENTA,
    TTY_COLOR_YELLOW,
    TTY_COLOR_BRIGHT_RED,
    TTY_COLOR_BRIGHT_GREEN,
    TTY_COLOR_BRIGHT_BLUE,
    TTY_COLOR_BRIGHT_CYAN,
    TTY_COLOR_BRIGHT_MAGENTA,
    TTY_COLOR_BRIGHT_YELLOW,
};

/**
 * Visual style of a character. Please note that not all terminals support all
 * of the attributes.
 */
struct tty_style {
    enum tty_color fg;
    enum tty_color bg;
    uint8_t flags;
};

void tty_putchar(uint32_t c);
void tty_setcursor(int row, int col);
void tty_reset(void);
void tty_clear(void);

#endif
