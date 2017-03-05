#include <mm/pages.h>

void reload_paging()
{
    __asm__ volatile ("mov %cr3, %rax");
    __asm__ volatile ("mov %rax, %cr3");
}
