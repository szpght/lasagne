#pragma once
extern void *_KERNEL_LMA;
extern void *_KERNEL_VMA;
#define KERNEL_LMA ((void*)&_KERNEL_LMA)
#define KERNEL_VMA ((void*)&_KERNEL_VMA)