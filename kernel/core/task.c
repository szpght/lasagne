#include <task.h>
#include <string.h>
#include <mm/memory_map.h>
#include <mm/alloc.h>
#include <printk.h>
#include <ds.h>


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
        preempt_sys();
    }
}

void initialize_tasks()
{
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
    uint64_t *entry = thread->rsp - 8;
    uint64_t *rflags = thread->rsp - 2 * 8;
    *entry = main;
    *rflags = RFLAGS_IF;
    thread->rsp -= 17 * 8; // all general registers w/o rsp + rflags + rip
    thread->state = THREAD_RUNNING;
    thread->task = task;

    CLIST_ADD(task->threads, thread);
    return thread;
}

void preempt_sys()
{
    struct thread *old_thread = current_thread;
    current_thread = current_thread->next;
    switch_task_sys(&old_thread->rsp, current_thread->rsp);
}
