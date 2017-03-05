#pragma once
extern void *_KERNEL_LMA;
extern void *_KERNEL_PHYS;
extern void *_KERNEL_VMA;
extern void *_KERNEL_END;
extern struct pml4te _pml4t;
#define KERNEL_LMA ((void*)&_KERNEL_LMA)
#define KERNEL_PHYS ((void*)0xFFFF800000000000)
#define KERNEL_VMA ((void*)&_KERNEL_VMA)

/* this symbol is used by frame allocator, so it must be defined
 * and initialized before allocator initialization
 */
extern void *KERNEL_END;
#define KERNEL_END_PHYS (KERNEL_END - KERNEL_VMA + KERNEL_PHYS)
#define PML4T (&_pml4t)
//pml4t in the form of pointer array
#define PML4T_P ((void **)PML4T)