#pragma once
#include <stdbool.h>
#include <stdint.h>

#define RFLAGS_IF (1 << 9)
#define DEFAULT_STACK_SIZE 8192

#define TSS_AVAILABLE 9;
#define TSS_BUSY 11;

#define CODE_SEGMENT 0x08
#define DATA_SEGMENT 0x10
#define USER_CODE_SEGMENT 0x1b
#define USER_DATA_SEGMENT 0x23

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


enum thread_state {
    THREAD_RUNNING,
    THREAD_WAITING,
    THREAD_RETURNED
};


struct thread {
    enum thread_state state;
    uint64_t *rsp;
    struct task *task;
    struct thread *next;
    struct thread *prev;
};


struct task {
    int pid;
    char *name;
    uintptr_t memory;
    struct thread *main_thread;
    struct thread *threads;
    struct task *next;
    struct task *prev;
};

void initialize_tasks();
void initialize_task(struct task *task, char *name, bool userspace, void *main);
struct task *create_task(char *name, bool userspace, void *main);
struct thread *create_kernel_thread(struct task *task, void *main);
struct thread *create_usermode_thread(struct task *task, void *main, uint64_t stack);
void preempt_int();
void create_usermode_task();

extern void load_tss();
extern void switch_task_sys(uint64_t **old_rsp, uint64_t *new_rsp);
extern void switch_task_int(uint64_t **old_rsp, uint64_t *new_rsp);
