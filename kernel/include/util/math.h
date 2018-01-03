#pragma once

static inline long ceil_to_power_of_two(long number)
{
    int a = __builtin_ctzl(number);
    int b = 63 - __builtin_clzl(number);
    if (a != b) {
        b += 1;
    }
    return 1 << b;
}

static inline int log2(long number)
{
    return __builtin_ctzl(number);
}
