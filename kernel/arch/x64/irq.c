#include <irq.h>
#include <io/ports.h>
#include <mm/alloc.h>
#include <printk.h>
#include <task.h>

struct idt_entry idt[INT_VECTORS_NUMBER];
struct idt_handler idt_handler[INT_VECTORS_NUMBER];
long spurious_interrupts_count;

static void set_idt()
{
    load_idt(sizeof(struct idt_entry) * INT_VECTORS_NUMBER, idt);
}

void initialize_irq()
{
    initialize_pic();
    create_idt();
    set_idt();
    set_handlers();
    enable_irq();
}

void initialize_pic()
{
    // read current masks, no idea if necessary
    inb(PIC1_DATA);
    inb(PIC2_DATA);

    // start initialization sequence
    outb(PIC1_CMD, PIC_INIT);
    outb(PIC2_CMD, PIC_INIT);

    // set irq number offsets
    outb(PIC1_DATA, PIC1_OFFSET);
    outb(PIC2_DATA, PIC2_OFFSET);

    // set PIC1 to master and PIC2 to slave
    outb(PIC1_DATA, PIC_MASTER);
    outb(PIC2_DATA, PIC_SLAVE);

    // set 8086 mode, whatever it is
    outb(PIC1_DATA, PIC_8086);
    outb(PIC2_DATA, PIC_8086);

    // set masks to disable all interrupts
    outb(PIC1_DATA, 0xFF); // 0xFD for keyboard
    outb(PIC2_DATA, 0xFF);
}

void pic_flip_irq(int irq)
{
    char port;
    if (irq >= 8) {
        port = PIC2_DATA;
        irq -= 8;
    }
    else {
        port = PIC1_DATA;
    }
    char mask = inb(port);
    mask ^= 1 << irq;
    outb(port, mask);
}

void compile_idt(struct idt_entry *dest, struct idt_model *src)
{
    dest->offset_low = src->offset;
    dest->offset_middle = src->offset >> 16;
    dest->offset_high = src->offset >> 32;
    dest->selector = src->selector;
    dest->ist = src->ist;
    dest->type = src->type;
    dest->dpl = src->dpl;
    dest->present = src->present;
    dest->reserved = 0;
}

void load_idt(uint16_t size, void *idt)
{
    struct idtr idtr = {
        .limit = size - 1,
        .offset = idt
    };
    _load_idt(&idtr);
}

void create_idt()
{
    struct idt_model idt_model = {
        .selector = 0x08,
        .ist = 0,
        .type = 0xE,
        .dpl = 0,
        .present = 0
    };
    for (int i = 0; i < INT_VECTORS_NUMBER; ++i) {
        idt_model.offset = (uint64_t)interrupt_wrapper + INT_WRAPPER_ALIGN * i;
        compile_idt(idt + i, &idt_model);
    }
}

void set_irq_handler(int irq_number, void *address, uint64_t flags)
{
    idt_handler[irq_number].address = address;
    idt_handler[irq_number].flags = flags;
    idt[irq_number].present = 1;
    if (flags & INT_HANDLER_USER) {
        idt[irq_number].dpl = 3;
    }
    else {
        idt[irq_number].dpl = 0;
    }
}

void reset_irq_handler(int irq_number)
{
    idt[irq_number].present = 0;
}

void generic_exception_handler(struct irq_state *regs, uint64_t error_code)
{
    static char *exception_name[31] = {
        "Divide-by-zero Error",
        "Debug",
        "Non-maskable Interrupt",
        "Breakpoint",
        "Overflow",
        "Bound Range Exceeded",
        "Invalid Opcode",
        "Device Not Available",
        "Double Fault",
        "Coprocessor Segment Overrun",
        "Invalid TSS",
        "Segment Not Present",
        "Stack-Segment Fault",
        "General Protection Fault",
        "Page Fault",
        "Reserved",
        "x87 Floating-Point Exception",
        "Alignment Check",
        "Machine Check",
        "SIMD Floating-Point Exception",
        "Virtualization Exception",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Security Exception"
    };

    disable_irq();
    void *irq_stack_frame = &(regs->irq_stack_frame);
    printk("Exception #%ld occured: %s\n", regs->irq, exception_name[regs->irq]);
    if (idt_handler[regs->irq].flags & INT_HANDLER_ERRORCODE) {
        printk("Error code %lx\n", error_code);
        irq_stack_frame += 8;
    }
    struct irq_stack_frame *irq_frame = irq_stack_frame;
    printk("RAX = %lx    RBX = %lx    RCX = %lx\n", regs->rax, regs->rbx, regs->rcx);
    printk("RDX = %lx    RSI = %lx    RDI = %lx\n", regs->rdx, regs->rsi, regs->rdi);
    printk("RBP = %lx     R8 = %lx     R9 = %lx\n", regs->rbp, regs->r8, regs->r9);
    printk("R10 = %lx    R11 = %lx    R12 = %lx\n", regs->r10, regs->r11, regs->r12);
    printk("R13 = %lx    R14 = %lx    R15 = %lx\n", regs->r13, regs->r14, regs->r15);
    printk("RIP = %lx     CS = %lx\n", irq_frame->rip, irq_frame->cs);
    printk("RSP = %lx     SS = %lx\n", irq_frame->rsp, irq_frame->ss);
    printk("RFLAGS = %lx\n", irq_frame->rflags);
    printk("CR2 = %lx\n", get_cr2());
    printk("System halted\n");

    for(;;) {
        __asm__ volatile ("hlt");
    }
}

void set_handlers()
{
    for (int i = 0 ; i <= 7; ++i) {
        set_irq_handler(i, generic_exception_handler, 0);
    }
    for (int i = 8 ; i <= 14; ++i) {
        set_irq_handler(i, generic_exception_handler, INT_HANDLER_ERRORCODE);
    }
    for (int i = 16 ; i <= 79; ++i) {
        set_irq_handler(i, generic_exception_handler, 0);
    }

   set_irq_handler(0x27, spurious_interrupt_handler, 0);
}

void irq_eoi()
{
    outb(PIC1_CMD, PIC_EOI);
}

void spurious_interrupt_handler()
{
    spurious_interrupts_count += 1;
}
