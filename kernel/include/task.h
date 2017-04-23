#pragma once
#include <stdbool.h>
#include <stdint.h>

#define RFLAGS_IF (1 << 9)
#define DEFAULT_STACK_SIZE 8192

#define TSS_AVAILABLE 9;
#define TSS_BUSY 11;

extern struct tss_descriptor tss_descriptor;

struct tss {
    uint32_t reserved;
    uint64_t* rsp0;
    uint64_t* rsp1;
    uint64_t* rsp2;
    uint64_t reserved2;
    uint64_t* ist[7];
    uint64_t reserved3;
    uint16_t reserved4;
    uint16_t iomap_base_addr;
} __attribute__((packed));


struct tss_descriptor {
    uint16_t limit15_0;
    uint32_t base_addr23_0 : 24;
    uint32_t type : 7; // includes reserved 0 bit and dpl
    uint32_t present : 1;
    uint8_t limit19_16 : 4;
    uint8_t avl : 4; // includes 2 reserved 0 bits and 1 byte granularity
    uint8_t base31_24;
    uint32_t base63_32;
    uint32_t zero;
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
    uint64_t *rsp;
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
void preempt_int();
void setup_tss();

void switch_task_sys(uint64_t **old_rsp, uint64_t *new_rsp);
void switch_task_int(uint64_t **old_rsp, uint64_t *new_rsp);
void leave_interrupt_handler();
