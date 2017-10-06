#include <types.h>
#include <mm/memory_map.h>
#include <io/tty.h>
#include <io/ports.h>

static struct {
    int row;
    int col;
    int width;
    int height;
    int buffer_row;
    int buffer_height;
    int tab;
    struct tty_style style;
    uint16_t buffer[80 * 30];
} tty_state;

static uint16_t *tty_buffer = KERNEL_VMA + 0xb8000;

static void buffer_row_up()
{
    tty_state.buffer_row = (tty_state.buffer_row + 1) % tty_state.buffer_height;
    uint16_t *row = tty_state.buffer + tty_state.width * tty_state.buffer_row;
    for (int i = 0; i < tty_state.width; ++i) {
        row[i] = 0;
    }
}

static void line_up()
{
    buffer_row_up();
    if (tty_state.row < tty_state.height - 1) {
        tty_state.row += 1;
        return;
    }

    // write shifted text to video memory
    int start_position = (tty_state.buffer_row - tty_state.height + 1) % tty_state.buffer_height;
    if (start_position < 0) {
        start_position += tty_state.buffer_height;
    }
    uint16_t *buffer = tty_buffer;
    int row = 0;
    for (int i = start_position; i != tty_state.buffer_row; i = (i + 1) % tty_state.buffer_height) {
        for (int j = 0; j < tty_state.width; ++j) {
            buffer[tty_state.width * row + j] = tty_state.buffer[tty_state.width * i + j];
        }
        ++row;
    }
    for (int i = 0; i < tty_state.width; ++i) {
    tty_buffer[tty_state.width * row + i] = 0;
    }
}

static void special_char(uint32_t c)
{
    switch (c) {
    case '\n':
        line_up();
    // fall through
    case '\r':
        tty_state.col = 0;
        break;
    case '\t':
        tty_state.col += tty_state.tab - tty_state.col % tty_state.tab;

        if (tty_state.col + 1 > tty_state.width) {
            tty_state.col = tty_state.col % tty_state.width;
            line_up();
        }
        break;
    }
}

static void put_char(uint32_t c)
{
    if (tty_state.col == tty_state.width) {
        line_up();
        tty_state.col = 0;
    }
    uint16_t ch = tty_state.style.fg << 8 | tty_state.style.bg << 12 | c;
    tty_buffer[tty_state.row * tty_state.width + tty_state.col] = ch;
    tty_state.buffer[tty_state.buffer_row * tty_state.width + tty_state.col] = ch;
    tty_state.col += 1;
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

__init void tty_initialize()
{
    tty_state.row = tty_state.col = 0;
    tty_state.buffer_row = 0;
    tty_state.buffer_height = 30;
    tty_state.width = 80;
    tty_state.height = 25;
    tty_state.tab = 8;
    tty_state.style.fg = TTY_COLOR_WHITE;
    tty_state.style.bg = TTY_COLOR_BLACK;
    tty_state.style.flags = 0;

    int buffer_size = tty_state.width * tty_state.buffer_height;
    for (int i = 0; i < buffer_size; ++i) {
        tty_state.buffer[i] = 0;
    }
    tty_clear();
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

void tty_set_tab_size(int size)
{
    if (size > 0 && size < tty_state.width) {
        tty_state.tab = size;
    }
}

void tty_clear()
{
    buffer_row_up();
    int size = tty_state.width * tty_state.height;
    for (int i = 0; i < size; ++i) {
        tty_buffer[i] = 0;
    }
    tty_state.col = 0;
    tty_state.row = 0;
}
