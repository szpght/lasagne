#pragma once

// beginning of kernel memory in physical address space
extern void *_KERNEL_LMA;
#define KERNEL_LMA ((void*)&_KERNEL_LMA)

// offset of kernel memory in virtual address space (-2 GiB)
extern void *_KERNEL_VMA;
#define KERNEL_VMA ((void*)&_KERNEL_VMA)

// end of kernel memory in virtual address space (-2 GiB)
extern void *_KERNEL_END;

/* this symbol stores virtual address of end of kernel tweaked
 * to contain boot time allocations
 * it is used by frame allocator, so it must be defined
 * and initialized before allocator initialization
 */
extern void *KERNEL_END;

// offset where kernel physical memory is mapped
#define KERNEL_PHYS ((void*)0xFFFF800000000000)

// end of kernel memory where physical memory is mapped
#define KERNEL_END_PHYS (KERNEL_END - KERNEL_VMA + KERNEL_PHYS)

extern struct pml4te _pml4t;

#define PML4T (&_pml4t)
//pml4t in the form of pointer array
#define PML4T_P ((void **)PML4T)