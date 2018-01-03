#pragma once
#include <stdint.h>

static inline void flip_bit(uint8_t *buffer, int bit)
{
    int index = bit / 8;
    int offset = bit % 8;
    buffer[index] ^= (1 << offset);
}

static inline uint8_t get_bit(uint8_t *buffer, int bit)
{
    int index = bit / 8;
    int offset = bit % 8;
    return (buffer[index] >> (offset)) & 1;
}

static inline void set_bit(uint8_t *buffer, int bit)
{
    int index = bit / 8;
    int offset = bit % 8;
    buffer[index] |= (1 << offset);
}

static inline void reset_bit(uint8_t *buffer, int bit)
{
    int index = bit / 8;
    int offset = bit % 8;
    buffer[index] &= ~(1 << offset);
}
