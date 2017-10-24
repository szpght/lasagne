#pragma once
#include <stdint.h>

#define SYSCALL_INT 0x30

#define SYSCALL_BY_INTERRUPT 0ULL
#define SYSCALL_BY_SYSCALL 1ULL

typedef uint64_t syscall_type_t;

void initialize_syscalls();

extern void syscall_fast_handler();
