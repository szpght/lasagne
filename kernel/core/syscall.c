#include <syscall.h>
#include <mm/memory_map.h>
#include <irq.h>
#include <printk.h>
#include <task.h>
#include <assert.h>

static void syscall_int_handler(struct irq_state *registers);

void sys_yield()
{
    preempt_int();
}

void sys_printk(char *template, uint64_t arg1, uint64_t arg2,
    uint64_t arg3, uint64_t arg4, uint64_t arg5)
{
    printk(template, arg1, arg2, arg3, arg4, arg5);
}

static void *syscall_table[] = 
{
    sys_yield,
    sys_printk
};

static const int syscall_count =
    sizeof(syscall_table)/sizeof(*syscall_table);

__init void initialize_syscalls()
{
    set_irq_handler(SYSCALL_INT, syscall_int_handler,
        INT_HANDLER_RETVAL | INT_HANDLER_USER);
}

void syscall_int_handler(struct irq_state *registers)
{
    uint64_t syscall_number = registers->rax;
    if (syscall_number >= syscall_count) {
        printk("WARNING non-existent syscall #%ld called\n", syscall_number);
        return;
    }
    uint64_t (*handler)(uint64_t, uint64_t, uint64_t) = syscall_table[syscall_number];
    uint64_t result = handler(registers->rdi, registers->rsi,
        registers->rdx);
    registers->rax = result;
}