#ifndef KERNEL_IO_H
#define KERNEL_IO_H

typedef struct iodevice iodevice;
struct iodevice {
    int (*write)(char *buf, size_t n);
};

int read();

#endif