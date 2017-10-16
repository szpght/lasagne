#pragma once

#define SYSCALL_INT 0x30

void initialize_syscalls();

extern void syscall_fast_handler();
