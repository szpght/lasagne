#include <types.h>
#include <memory_map.h>
#include <io/tty.h>

static struct {
    int row;
    int col;
    int width;
    int height;
    int tab;
    struct tty_style style;
} tty_state;

static uint16_t *tty_buffer = KERNEL_VMA + 0xb8000;

/* TODO: incomplete implementation for debugging purposes */
void tty_putchar(uint32_t c)
{
    uint16_t ch;

    /* non-ASCII characters not yet implemented */
    if (c >= 0xff)
        return;

    if (c < 0x20)
        switch (c) {
        case '\n':
            tty_state.row = (tty_state.row + 1) % tty_state.height;
            /* fall-through */
        case '\r':
            tty_state.col = 0;
            break;
        case '\t':
            if (tty_state.col % tty_state.tab == 0)
                tty_state.col += 8;
            else
                while (tty_state.col % tty_state.tab)
                    tty_state.col++;

            /* FIXME: let me explain this ugliness... */
            if (tty_state.col + 1 > tty_state.width) {
                tty_state.col = tty_state.col % tty_state.width;
                tty_state.row = (tty_state.row + 1) % tty_state.height;
            }
            break;
        }

    else {
        ch = tty_state.style.fg << 8 | tty_state.style.bg << 12 | c;
        tty_buffer[tty_state.row * tty_state.width + tty_state.col] = ch;
        tty_state.col = (tty_state.col + 1) % tty_state.width;
        if (tty_state.col == 0)
            tty_putchar('\n');
    }

}

void tty_initialize()
{
    tty_state.row = tty_state.col = 0;
    tty_state.width = 80;
    tty_state.height = 25;
    tty_state.tab = 8;
    tty_state.style.fg = TTY_COLOR_WHITE;
    tty_state.style.bg = TTY_COLOR_BLACK;
    tty_state.style.flags = 0;
}

void tty_set_color(enum tty_color foreground, enum tty_color background)
{
    tty_state.style.fg = foreground;
    tty_state.style.bg = background;
}

void tty_putstring(char *string)
{
    while (*string) {
        tty_putchar(*string);
        ++string;
    }
}