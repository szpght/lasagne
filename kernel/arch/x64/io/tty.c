#include <types.h>
#include <mm/memory_map.h>
#include <io/tty.h>
#include <io/ports.h>
#include <string.h>

static struct {
    int row;
    int col;
    int width;
    int height;
    int buffer_row;
    int buffer_height;
    int tab;
    struct tty_style style;
    uint16_t buffer[30][80];
} tty;

static uint16_t* const tty_buffer = KERNEL_VMA + 0xb8000;

static void buffer_row_up()
{
    tty.buffer_row = (tty.buffer_row + 1) % tty.buffer_height;
    memset(tty.buffer[tty.buffer_row], 0, sizeof(*tty.buffer));
}

static void line_up()
{
    buffer_row_up();
    if (tty.row < tty.height - 1) {
        tty.row += 1;
        return;
    }

    // write shifted text to video memory
    int start_line = (tty.buffer_row - tty.height + 1) % tty.buffer_height;
    if (start_line < 0) {
        start_line += tty.buffer_height;
    }

    int row = 0;
    for (int i = start_line; i != tty.buffer_row; i = (i + 1) % tty.buffer_height) {
        memcpy(&tty_buffer[tty.width * row], tty.buffer[i], sizeof(*tty.buffer));
        ++row;
    }

    memset(tty_buffer + tty.width * row, 0, tty.width * sizeof(*tty_buffer));
}

static void special_char(uint32_t c)
{
    switch (c) {
    case '\n':
        line_up();
    // fall through
    case '\r':
        tty.col = 0;
        break;
    case '\t':
        tty.col += tty.tab - tty.col % tty.tab;

        if (tty.col + 1 > tty.width) {
            tty.col = tty.col % tty.width;
            line_up();
        }
        break;
    }
}

static void put_char(uint32_t c)
{
    if (tty.col == tty.width) {
        line_up();
        tty.col = 0;
    }
    uint16_t ch = tty.style.fg << 8 | tty.style.bg << 12 | c;
    tty_buffer[tty.row * tty.width + tty.col] = ch;
    tty.buffer[tty.buffer_row][tty.col] = ch;
    tty.col += 1;
}

__init void tty_initialize()
{
    tty.row = tty.col = 0;
    tty.buffer_row = 0;
    tty.buffer_height = 30;
    tty.width = 80;
    tty.height = 25;
    tty.tab = 8;
    tty.style.fg = TTY_COLOR_WHITE;
    tty.style.bg = TTY_COLOR_BLACK;
    tty.style.flags = 0;

    memset(tty.buffer, 0, sizeof(tty.buffer));
    tty_clear();
}

void tty_putchar(uint32_t c)
{
    // multibyte characters not implemented
    if (c > 0xff) {
        return;
    }

    if (c < 0x20) {
        special_char(c);
    }
    else {
        put_char(c);
    }
}

void tty_set_color(enum tty_color foreground, enum tty_color background)
{
    tty.style.fg = foreground;
    tty.style.bg = background;
}

void tty_putstring(char *string)
{
    while (*string) {
        tty_putchar(*string);
        ++string;
    }
}

void tty_set_tab_size(int size)
{
    if (size > 0 && size < tty.width) {
        tty.tab = size;
    }
}

void tty_clear()
{
    buffer_row_up();
    int size = tty.width * tty.height * sizeof(*tty_buffer);
    memset(tty_buffer, 0, size);
    tty.col = 0;
    tty.row = 0;
}
