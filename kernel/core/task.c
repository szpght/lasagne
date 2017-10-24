#include <task.h>
#include <string.h>
#include <mm/memory_map.h>
#include <mm/alloc.h>
#include <mm/pages.h>
#include <printk.h>
#include <ds.h>
#include <irq.h>
#include <io/io.h>

struct tss tss;
struct thread *current_thread;

static struct task kernel_task;
static struct thread kernel_main_thread;
static struct thread *kernel_idle_thread;

static struct task *current_task;

static pid_t next_pid = 1;
static pid_t next_tid = 1;

__init static void setup_tss()
{
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
}

__init static void init_kernel_main_thread()
{
    kernel_main_thread.tid = 0;
    kernel_main_thread.state = THREAD_RUNNING;
    kernel_main_thread.task = &kernel_task;
}

__init static void init_kernel_idle_thread()
{
    kernel_idle_thread = create_kernel_thread(&kernel_task, idle_thread);
    kernel_idle_thread->state = THREAD_WAITING;
}

__init static void init_kernel_task()
{
    kernel_task.pid = 0;
    kernel_task.name = "kernel";
    kernel_task.memory = 0;
    init_kernel_main_thread();
    init_kernel_idle_thread();
    kernel_task.main_thread = &kernel_main_thread;
    CLIST_ADD(kernel_task.threads, &kernel_main_thread);
    CLIST_ADD(current_task, &kernel_task);
}

void do_sth()
{
    int counter = 0;
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
    create_usermode_task(1);
    create_usermode_task(0);
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
    thread->tid = next_tid++;

    // add new thread as next element of list
    struct thread *tmp = task->threads;
    if (tmp) {
        CLIST_ADD(tmp, thread);
    }
    else {
        CLIST_ADD(task->threads, thread);
    }
    return thread;
}

static uintptr_t* get_address_space_ptr()
{
    struct pt_entries e = ptes(0);
    return e.entries[0];
}

static void new_address_space()
{
    uintptr_t *ptr = get_address_space_ptr();
    *ptr = 0;
    reload_paging();
}

static void set_address_space(uintptr_t pdpt)
{
    uintptr_t *old = get_address_space_ptr();
    if (*old != pdpt) {
        *old = pdpt;
        reload_paging();
    }
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
extern void usermode_function2();

void create_usermode_task(int xd)
{
    new_address_space();
    void *virtual_memory_start = (void*) (1024 * 1024);
    map_range(virtual_memory_start, 4096, MAP_RW | MAP_EXE | MAP_USER);
    if (xd) {
        memcpy(virtual_memory_start, usermode_function, 1000);
    }
    else {
        memcpy(virtual_memory_start, usermode_function2, 1000);
    }
    

    void *stack = (void*) (2 * 1024 * 1024);
    map_range(stack, 4096, MAP_RW | MAP_EXE | MAP_USER);

    struct task *task = kalloc(sizeof(*task));
    task->next = task->prev = NULL;
    task->threads = NULL;
    task->name = "user_process";
    task->pid = next_pid++;
    task->memory = ptes(0).entries[0][0];
    task->main_thread = create_usermode_thread(task, virtual_memory_start, stack + 4096);

    // add new task as next element of list
    struct task *tmp = current_task;
    if (tmp) {
        CLIST_ADD(tmp, task);
    }
    else {
        CLIST_ADD(current_task, task);
    }
}

void set_current_kernel_stack(uintptr_t stack)
{
    tss.rsp0 = stack;
}

static void switch_environment(struct thread *old_thread, struct thread *new_thread)
{
    if (old_thread == new_thread) {
        return;
    }

    wrmsr(IA32_KERNEL_GS_BASE, (uint64_t) new_thread);
    uintptr_t memory = new_thread->task->memory;
    if (memory) {
        set_address_space(memory);
    }
    set_current_kernel_stack(new_thread->stack_top);
}

static struct thread *schedule()
{
    struct thread *old_thread = current_thread;
    struct task *old_task = current_task;
    struct task *new_task = current_task;

    // find next runnable thread
    do {
        LIST_NEXT(new_task);
        struct thread *first_thread = new_task->threads;

        do {
            LIST_NEXT(new_task->threads);
            if (new_task->threads->state == THREAD_RUNNING) {
                return new_task->threads;
            }

        } while(new_task->threads != first_thread);
    } while(new_task != old_task);

    // if no runnable thread found, switch to idle thread
    return kernel_idle_thread;
}

void preempt_int()
{
    struct thread *old_thread = current_thread;
    current_thread = schedule();
    current_task = current_thread->task;
    switch_environment(old_thread, current_thread);
    switch_task_int(&old_thread->rsp, current_thread->rsp);
}

void preempt_syscall()
{
    struct thread *old_thread = current_thread;
    current_thread = schedule();
    current_task = current_thread->task;
    switch_environment(old_thread, current_thread);
    switch_task_sys(&old_thread->rsp, current_thread->rsp);
}

pid_t get_current_task_pid()
{
    return current_thread->task->pid;
}
