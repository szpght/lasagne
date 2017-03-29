#include <task.h>
#include <string.h>
#include <mm/memory_map.h>
#include <mm/alloc.h>
#include <printk.h>
#include <ds.h>
#include <irq.h>

struct tss tss;


struct task kernel_task;
struct task *tasks;
struct thread *current_thread;

void do_sth()
{
    static int counter = 0;
    while (1) {
        for (int i = 0; i < 50000000; ++i) {
            __asm__ volatile ("nop");
        }
        printk("B %d\n", ++counter);
        __asm__ volatile ("int $48");
    }
}

void initialize_tasks()
{
    setup_tss();
    initialize_task(&kernel_task, "kernel", false, NULL);
    current_thread = kernel_task.main_thread;
    create_thread(&kernel_task, do_sth);
}

void initialize_task(struct task *task, char *name, bool userspace, void *main)
{
    task->name = name;
    task->userspace = userspace;
    task->main_thread = create_thread(task, main);
    CLIST_ADD(task->threads, task->main_thread);
    CLIST_ADD(tasks, task);
}

struct task *create_task(char *name, bool userspace, void *main)
{
    struct task *task = kalloc(sizeof *task);
    initialize_task(task, name, userspace, main);
    return task;
}

struct thread *create_thread(struct task *task, void *main)
{
    struct thread *thread = kalloc(sizeof *thread);

    thread->rsp = (kalloc(DEFAULT_STACK_SIZE) + DEFAULT_STACK_SIZE);
    *(--thread->rsp) = 0x10; // ss
    uint64_t *place_for_rsp = --thread->rsp; // rsp
    *(--thread->rsp) = RFLAGS_IF; // rflags
    *(--thread->rsp) = 0x8; // cs
    *(--thread->rsp) = (uint64_t) switch_task_int_return;
    *(--thread->rsp) = 0; // interrupt number
    for (int i = 0; i < 15; ++i) {
        *(--thread->rsp) = 0;
    }
    *(--thread->rsp) = (uint64_t) main;
    *place_for_rsp = (uint64_t) thread->rsp;

    thread->state = THREAD_RUNNING;
    thread->task = task;

    CLIST_ADD(task->threads, thread);
    return thread;
}

void preempt_sys()
{
}

void setup_tss()
{
    disable_irq();
    struct tss_descriptor *tss_desc = &_tss_descriptor + (uint64_t) KERNEL_VMA;
    tss_desc->limit15_0 = sizeof tss;
    tss_desc->base_addr23_0 = (uint64_t)(&tss) & 0xFFFFFF;
    // tss must be available, it will automatically change to busy when loaded
    tss_desc->type = TSS_AVAILABLE;
    tss_desc->limit19_16 = 0;
    tss_desc->avl = 1;
    tss_desc->base31_24 = ((uint64_t)(&tss) >> 24) & 0xFF;
    tss_desc->base63_32 = ((uint64_t)(&tss) >> 32) & 0xFFFFFFFF;
    tss_desc->zero = 0;
    tss_desc->present = 1;
    __asm__ volatile("push %rax");
    __asm__ volatile("movw $0x28, %ax");
    __asm__ volatile("ltr %ax");
    __asm__ volatile("pop %rax");
    enable_irq();
}

void set_current_kernel_stack(void *stack)
{
    tss.rsp0 = stack;
}

void preempt_int()
{
    struct thread *old_thread = current_thread;
    current_thread = current_thread->next;
    switch_task_int(&old_thread->rsp, current_thread->rsp);
}
