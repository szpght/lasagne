#pragma once
#include <stdbool.h>
#include <stdint.h>

#define RFLAGS_IF 1 << 9
#define DEFAULT_STACK_SIZE 8192


struct tss {
    uint32_t reserved;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved2;
    uint64_t ist[7];
    uint64_t reserved3;
    uint16_t reserved4;
    uint16_t iomap_base_addr;
} __attribute__((packed));


struct registers {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rflags;
    uint64_t rip;
} __attribute__((packed));


enum thread_state {
    THREAD_RUNNING,
    THREAD_WAITING,
    THREAD_RETURNED
};


struct thread {
    enum thread_state state;
    struct registers registers;
    uint64_t rsp;
    struct task *task;
    struct thread *next;
    struct thread *prev;
};


struct task {
    char *name;
    bool userspace;
    struct thread *main_thread;
    struct thread *threads;
    struct task *next;
    struct task *prev;
};

void initialize_tasks();
void initialize_task(struct task *task, char *name, bool userspace, void *main);
struct task *create_task(char *name, bool userspace, void *main);
struct thread *create_thread(struct task *task, void *main);
void preempt_sys();

//void switch_task_sys(uint64_t *old_rsp, uint64_t new_rsp);
void switch_task_sys(uint64_t *old_rsp, uint64_t new_rsp);
