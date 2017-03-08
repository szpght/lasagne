#include <irq.h>
#include <io/ports.h>
#include <mm/alloc.h>

struct idt_entry idt[INT_VECTORS_NUMBER];
struct idt_handler idt_handler[INT_VECTORS_NUMBER];

void syscall_stub()
{
    printk("Syscall called\n");
}

void initialize_irq()
{
    initialize_pic();
    create_idt();
    load_idt(sizeof(struct idt_entry) * INT_VECTORS_NUMBER, idt);
    enable_irq();
    set_irq_handler(0x30, syscall_stub, 0);
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
        .present = 1
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
}
