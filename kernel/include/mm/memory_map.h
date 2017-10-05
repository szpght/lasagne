#pragma once
#include <stdint.h>

// offset of kernel memory in virtual address space (-2 GiB)
extern void *_KERNEL_VMA;
#define KERNEL_VMA ((void*)&_KERNEL_VMA)

/* this symbol stores virtual address of end of kernel tweaked
 * to contain boot time allocations
 * it is used by frame allocator, so it must be defined
 * and initialized before allocator initialization
 */
extern uint64_t KERNEL_END;

#define MAP_MAX 32

#define __init __attribute__ ((section (".text.init")))
#define __initdata __attribute__ ((section (".data.init")))

struct area {
    uint64_t begin;
    uint64_t end;
};

struct memory_map {
    int count;
    struct area area[MAP_MAX];
    uint64_t physical_end;
};

extern struct memory_map mem_map;

void mem_map_add_area(uint64_t begin, uint64_t end);
void mem_map_sanitize();
