#pragma once
extern void *_KERNEL_LMA;
extern void *_KERNEL_PHYS;
extern void *_KERNEL_VMA;
extern void *_KERNEL_END;
#define KERNEL_LMA ((void*)&_KERNEL_LMA)
#define KERNEL_PHYS ((void*)0xFFFF800000000000)
#define KERNEL_VMA ((void*)&_KERNEL_VMA)
#define KERNEL_END ((void*)&_KERNEL_END)
#define KERNEL_END_PHYS (KERNEL_END - KERNEL_VMA + KERNEL_PHYS)