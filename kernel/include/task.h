#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <syscall.h>

#define RFLAGS_IF (1 << 9)
#define DEFAULT_STACK_SIZE 8192

#define TSS_AVAILABLE 9;
#define TSS_BUSY 11;

#define CODE_SEGMENT 0x08ULL
#define DATA_SEGMENT 0x10ULL
#define USER_DATA_SEGMENT 0x1bULL
#define USER_CODE_SEGMENT 0x23ULL

typedef uint64_t pid_t;

extern struct tss_descriptor tss_descriptor;
extern struct thread *current_thread;

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
    THREAD_IDLE
};


struct thread {
    uintptr_t user_rsp;
    uintptr_t stack_top;
    // contains 1 after 'syscall' instruction triggered syscall
    // used by syscall dispatcher to call correct preempt_* function
    syscall_type_t syscall_type;
    pid_t tid;
    uint64_t *rsp;
    enum thread_state state;
    struct task *task;
    struct thread *next;
    struct thread *prev;
};


struct task {
    pid_t pid;
    char *name;
    uintptr_t memory;
    struct thread *main_thread;
    struct thread *threads;
    struct task *next;
    struct task *prev;
};

void initialize_tasks();
struct thread *create_kernel_thread(struct task *task, void *main);
struct thread *create_usermode_thread(struct task *task, void *main, uint64_t stack);
void preempt_int();
void preempt_syscall();
void create_usermode_task();
pid_t get_current_task_pid();

extern void load_tss();
extern void switch_task_sys(uint64_t **old_rsp, uint64_t *new_rsp);
extern void switch_task_int(uint64_t **old_rsp, uint64_t *new_rsp);
extern void idle_thread();
