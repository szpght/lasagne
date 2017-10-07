#include <task.h>
#include <string.h>
#include <mm/memory_map.h>
#include <mm/alloc.h>
#include <printk.h>
#include <ds.h>
#include <irq.h>

struct tss tss;

struct task kernel_task;
struct thread kernel_main_thread;
struct task *tasks;
struct thread *current_thread;

__init static void setup_tss()
{
    disable_irq();
    tss_descriptor.limit15_0 = sizeof tss;
    tss_descriptor.base_addr23_0 = (uint64_t)(&tss) & 0xFFFFFF;
    // tss must be available, it will automatically change to busy when loaded
    tss_descriptor.type = TSS_AVAILABLE;
    tss_descriptor.limit19_16 = 0;
    tss_descriptor.avl = 1;
    tss_descriptor.base31_24 = ((uint64_t)(&tss) >> 24) & 0xFF;
    tss_descriptor.base63_32 = ((uint64_t)(&tss) >> 32) & 0xFFFFFFFF;
    tss_descriptor.zero = 0;
    tss_descriptor.present = 1;
    load_tss();
    enable_irq();
}

__init static void init_kernel_main_thread()
{
    kernel_main_thread.state = THREAD_RUNNING;
    kernel_main_thread.task = &kernel_task;
}

__init static void init_kernel_task()
{
    kernel_task.pid = 1;
    kernel_task.name = "kernel";
    kernel_task.memory = 0;
    init_kernel_main_thread();
    kernel_task.main_thread = &kernel_main_thread;
    CLIST_ADD(kernel_task.threads, &kernel_main_thread);
    CLIST_ADD(tasks, &kernel_task);
}

void do_sth()
{
    static int counter = 0;
    while (1) {
        for (int i = 0; i < 5000000; ++i) {
            __asm__ volatile ("nop");
        }
        printk("B %d\n", ++counter);
    }
}

__init void initialize_tasks()
{
    setup_tss();
    init_kernel_task();
    current_thread = kernel_task.main_thread;
    create_thread(&kernel_task, do_sth);
}

void initialize_task(struct task *task, char *name, bool userspace, void *main)
{
    (void)userspace;
    task->name = name;
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

    void* stack = kalloc(DEFAULT_STACK_SIZE);
    thread->rsp = (stack + DEFAULT_STACK_SIZE);
    *(--thread->rsp) = 0x10; // ss
    *(--thread->rsp) = (uint64_t) stack + DEFAULT_STACK_SIZE; // rsp
    *(--thread->rsp) = RFLAGS_IF; // rflags
    *(--thread->rsp) = 0x8; // cs
    *(--thread->rsp) = (uint64_t) main;
    *(--thread->rsp) = 0; // interrupt number
    for (int i = 0; i < 15; ++i) {
        *(--thread->rsp) = 0;
    }
    *(--thread->rsp) = (uint64_t) leave_interrupt_handler;
    *(--thread->rsp) = 0; // rbp

    thread->state = THREAD_RUNNING;
    thread->task = task;

    CLIST_ADD(task->threads, thread);
    return thread;
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
