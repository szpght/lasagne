#include <string.h>
#include <stdint.h>

void *memset(void *buffer, int c, size_t num)
{
    uint8_t *buf = (uint8_t*) buffer;
    uint8_t *end = buf + num;
    while (buf < end) {
        *buf = c;
        ++buf;
    }
    return buffer;
}

int memcmp(const void *ptr1, const void *ptr2, size_t num)
{
    const uint8_t *p1 = ptr1;
    const uint8_t *p2 = ptr2;
    for (size_t i = 0; i < num; ++i) {
        if (p1[i] != p2[i]) {
            return p1[i] - p2[i];
        }
    }
    return 0;
}

void *memcpy(void* dest, const void* src, size_t size)
{
    const uint8_t *s = src;
    uint8_t *d = dest;
    for (size_t i = 0; i < size; ++i) {
        d[i] = s[i];
    }
    return dest;
}

void *memmove(void* dest, const void* src, size_t size)
{
    const uint8_t *s = src;
    uint8_t *d = dest;
    if (d > s) {
        for (size_t i = size - 1; i > 0; --i) {
            d[i] = s[i];
        }
        d[0] = s[0];
    }
    else {
        for (size_t i = 0; i < size; ++i) {
            d[i] = s[i];
        }
    }
    return dest;
}