#include <stdint.h>
#include <stddef.h>
#include "../memory_sections.h"
#include <kernel/tty.h>

static struct {
    int row;
    int col;
    int width;
    int height;
    int tab;
} tty_state;

static uint16_t *tty_buffer = vmem_base + 0xb8000;

/* TODO: incomplete implementation for debugging purposes */
void
tty_putchar(uint32_t c, struct tty_style style)
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
            if (tty_state.col + 1 > tty_state.width) {
                tty_state.col = tty_state.col % tty_state.width;
                tty_state.row = (tty_state.row + 1) % tty_state.height;
            }
            break;
        }

    else {
        ch = c;
        tty_buffer[tty_state.row * tty_state.width + tty_state.col] = ch;
    }

}

void
tty_initialize(void)
{
    tty_state.row = tty_state.col = 0;
    tty_state.width = 80;
    tty_state.height = 24;
    tty_state.tab = 8;
}
