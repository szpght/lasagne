#include <printk.h>
#include <stdarg.h>
#include <io/tty.h>
#include <stdbool.h>
#include <io/io.h>

//void (*backend)(char) = tty_putchar;
void backend(char c)
{
    outb(0xe9, c);
    tty_putchar(c);
}

static void print_ulong(unsigned long n)
{
    if (n == 0) {
        backend('0');
    }
    int len = 0;
    char str[25] = { 0 };
    while (n) {
        str[len] = n % 10 + '0';
        ++len;
        n /= 10;
    }
    while (len > 0) {
        --len;
        backend(str[len]);
    }
}

static void print_long(long n)
{
    if (n < 0) {
        n = -n;
        backend('-');
    }
    print_ulong(n);
}

static void print_string(char *s)
{
    while (*s) {
        backend(*s);
        ++s;
    }
}

static void print_hex(unsigned long n, int len)
{
    print_string("0x");
    char str[16];
    for (int i = 0; i < len; ++i) {
        str[i] = n % 16;
        n /= 16;
    }
    int i = len;
    while (i --> 0) {
    //for (int i = len - 1; i >= 0; --i) {
        char c = str[i];
        if (c >= 0 && c <= 9) {
            backend(c + '0');
        }
        else {
            backend(c - 10 + 'A');
        }
    }
}

void printk(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    bool specifier = false;
    bool l = false;

    while (*fmt) {
        if (specifier) {
            specifier = false;
            switch (*fmt) {
            case 'd':
                if (l)
                    print_long(va_arg(args, long));
                else
                    print_long(va_arg(args, int));
                break;
            case 'u':
                if (l)
                    print_ulong(va_arg(args, unsigned long));
                else
                    print_ulong(va_arg(args, unsigned int));
                break;
            case 'x':
                if (l)
                    print_hex(va_arg(args, unsigned long), 16);
                else
                    print_hex(va_arg(args, unsigned int), 8);
                break;
            case 's':
                print_string(va_arg(args, char*));
                break;
            case '%':
                backend('%');
                break;
            case 'l':
                specifier = true;
                l = true;
            }
        }
        else if (*fmt == '%') {
            specifier = true;
            l = false;
        }
        else {
            backend(*fmt);
        }
        ++fmt;
    }
}
