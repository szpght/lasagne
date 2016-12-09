#include <stddef.h>
#include <kernel/tty.h>
#include "memory_sections.h"
#include "frames.h"

/* this is not a part of tty interface */
void tty_initialize(void);

void
platform_initialize(/* ... */)
{
    tty_initialize();
    mem_initialize_frame_management();
}
