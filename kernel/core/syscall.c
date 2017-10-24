#include <syscall.h>
#include <mm/memory_map.h>
#include <irq.h>
#include <printk.h>
#include <task.h>
#include <assert.h>
#include <io/io.h>

static void syscall_int_handler(struct irq_state *registers);

void sys_yield()
{
    if (current_thread->syscall_type == SYSCALL_BY_INTERRUPT) {
        preempt_int();
    }
    else if (current_thread->syscall_type == SYSCALL_BY_SYSCALL) {
        preempt_syscall();
    }
    else {
        assert(!"incorrect current_task->syscall_type value");
    }
}

void sys_printk(char *template, uint64_t arg1, uint64_t arg2,
    uint64_t arg3, uint64_t arg4, uint64_t arg5)
{
    printk(template, arg1, arg2, arg3, arg4, arg5);
}

void *syscall_table[] = 
{
    sys_yield,
    sys_printk
};

const unsigned int syscall_count =
    sizeof(syscall_table)/sizeof(*syscall_table);

__init static void initialize_int_handler()
{
    set_irq_handler(SYSCALL_INT, syscall_int_handler,
        INT_HANDLER_RETVAL | INT_HANDLER_USER);
}

__init static void initialize_fast_handler()
{
    uint64_t star = rdmsr(IA32_STAR);
    star &= 0xFFFFFFFFULL;
    star |= CODE_SEGMENT << 32;
    star |= (USER_DATA_SEGMENT - 8) << 48;
    wrmsr(IA32_STAR, star);
    wrmsr(IA32_LSTAR, (uint64_t) syscall_fast_handler);
    wrmsr(IA32_FMASK, RFLAGS_IF);
    uint64_t efer = rdmsr(IA32_EFER);
    efer |= 1;
    wrmsr(IA32_EFER, efer);
}

__init void initialize_syscalls()
{
    initialize_int_handler();
    initialize_fast_handler();
}

void bad_syscall(int syscall_number)
{
    printk("WARNING non-existent syscall #%ld called\n", syscall_number);
}

void syscall_int_handler(struct irq_state *registers)
{
    uint64_t syscall_number = registers->rax;
    if (syscall_number >= syscall_count) {
        bad_syscall(syscall_number);
        return;
    }
    current_thread->syscall_type = SYSCALL_BY_INTERRUPT;
    uint64_t (*handler)(uint64_t, uint64_t, uint64_t) = syscall_table[syscall_number];
    uint64_t result = handler(registers->rdi, registers->rsi,
        registers->rdx);
    registers->rax = result;
}
