#include <stddef.h>
#include "memory_sections.h"

/* this is not a part of tty interface */
void tty_initialize(void);

void
platform_initialize(/* ... */)
{
    tty_initialize();
}
