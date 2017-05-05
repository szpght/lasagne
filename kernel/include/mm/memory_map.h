#pragma once
#include <stdint.h>

// beginning of kernel memory in physical address space
extern void *_KERNEL_LMA;
#define KERNEL_LMA ((void*)&_KERNEL_LMA)

// offset of kernel memory in virtual address space (-2 GiB)
extern void *_KERNEL_VMA;
#define KERNEL_VMA ((void*)&_KERNEL_VMA)

// end of kernel memory in virtual address space (-2 GiB)
extern uint64_t KERNEL_END;

/* this symbol stores virtual address of end of kernel tweaked
 * to contain boot time allocations
 * it is used by frame allocator, so it must be defined
 * and initialized before allocator initialization
 */

// offset where kernel physical memory is mapped
#define KERNEL_PHYS ((void*)0xFFFF800000000000)

// end of kernel memory where physical memory is mapped
#define KERNEL_END_PHYS (KERNEL_END - KERNEL_VMA + KERNEL_PHYS)

extern struct pml4te _pml4t;

#define PML4T (&_pml4t)
//pml4t in the form of pointer array
#define PML4T_P ((void **)PML4T)

#define MAP_MAX 32

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
