#include <string.h>
#include <stdint.h>

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
