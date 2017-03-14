#pragma once
#include <stddef.h>

void *kalloc(size_t size);
void kfree(void *ptr);
void *kzalloc(size_t size);
