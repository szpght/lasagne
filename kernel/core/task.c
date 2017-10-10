#include <task.h>
#include <string.h>
#include <mm/memory_map.h>
#include <mm/alloc.h>
#include <mm/pages.h>
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
    create_kernel_thread(&kernel_task, do_sth);
    create_usermode_task();
}

void initialize_task(struct task *task, char *name, bool userspace, void *main)
{
    (void)userspace;
    task->name = name;
    task->main_thread = create_kernel_thread(task, main);
    CLIST_ADD(task->threads, task->main_thread);
    CLIST_ADD(tasks, task);
}

struct task *create_task(char *name, bool userspace, void *main)
{
    struct task *task = kalloc(sizeof *task);
    initialize_task(task, name, userspace, main);
    return task;
}

static struct thread *create_thread(struct task *task, void *main,
    uint64_t cs, uint64_t ss, uint64_t usermode_stack)
{
    struct thread *thread = kalloc(sizeof *thread);

    void* stack = kalloc(DEFAULT_STACK_SIZE);
    thread->rsp = (stack + DEFAULT_STACK_SIZE);
    thread->stack_top = (stack + DEFAULT_STACK_SIZE);
    *(--thread->rsp) = ss;
    if (!usermode_stack) {
        *(--thread->rsp) = (uint64_t) stack + DEFAULT_STACK_SIZE; // rsp
    }
    else {
        *(--thread->rsp) = usermode_stack;
    }
    *(--thread->rsp) = RFLAGS_IF; // rflags
    *(--thread->rsp) = cs;
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

struct thread *create_kernel_thread(struct task *task, void *main)
{
    return create_thread(task, main, CODE_SEGMENT, DATA_SEGMENT, 0);
}

struct thread *create_usermode_thread(struct task *task, void *main, uint64_t stack)
{
    return create_thread(task, main, USER_CODE_SEGMENT, USER_DATA_SEGMENT, stack);
}

extern void usermode_function();

void create_usermode_task()
{
    void *virtual_memory_start = (void*) (1024 * 1024);
    map_range(virtual_memory_start, 4096, MAP_RW | MAP_EXE | MAP_USER);
    memcpy(virtual_memory_start, usermode_function, 100);

    void *stack = (void*) (2 * 1024 * 1024);
    map_range(stack, 4096, MAP_RW | MAP_EXE | MAP_USER);

    struct task *task = kalloc(sizeof(*task));
    task->next = task->prev = task->threads = NULL;
    task->name = "user_process";
    task->pid = 2;
    task->memory = ptes(0).entries[0][0];
    task->main_thread = create_usermode_thread(task, virtual_memory_start, stack + 4096);
    LIST_ADD(tasks, task);
}

void set_current_kernel_stack(void *stack)
{
    tss.rsp0 = stack;
}

void preempt_int()
{
    struct thread *old_thread = current_thread;
    current_thread = tasks->threads;
    tasks = tasks->next;
    tasks->threads = tasks->threads->next;
    set_current_kernel_stack(current_thread->stack_top);
    switch_task_int(&old_thread->rsp, current_thread->rsp);
}
