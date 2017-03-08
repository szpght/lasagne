#pragma once
#include <stdint.h>

#define PIC1_CMD 0x20
#define PIC1_DATA 0x21
#define PIC2_CMD 0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI 0x20
#define PIC_INIT 0x11
#define PIC_MASTER 0x04
#define PIC_SLAVE 0x02
#define PIC_8086 0x01

#define PIC1_OFFSET 0x20
#define PIC2_OFFSET 0x28

#define INT_WRAPPER_ALIGN 8

#define INT_HANDLER_ERRORCODE (1 << 0)
#define INT_HANDLER_RETVAL (1 << 1)
#define INT_HANDLER_USER (1 << 2)

// this number must be consistnet with irq_asm.asm
#define INT_VECTORS_NUMBER 80

struct idtr {
    uint16_t limit;
    void *offset;
} __attribute__((packed));


struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type : 5;
    uint8_t dpl : 2;
    uint8_t present : 1;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__((packed));


struct idt_model
{
    uint64_t offset;
    uint16_t selector;
    uint8_t ist;
    uint8_t type;
    uint8_t dpl;
    uint8_t present;
};


struct idt_handler
{
    void *address;
    uint64_t flags;
} __attribute__((packed));


struct irq_state
{
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t irq;
} __attribute__((packed));


extern void interrupt_wrapper();

void initialize_irq();
void initialize_pic();
void compile_idt(struct idt_entry *dest, struct idt_model *src);
void load_idt(uint16_t size, void *idt);
void create_idt();
void create_handler_table();
void enable_irq();
void disable_irq();
void _load_idt(struct idtr *idtr);
void set_irq_handler(int irq_number, void *address, uint64_t flags);
void reset_irq_handler(int irq_number);
void generic_exception_handler(struct irq_state *registers, uint64_t error_code);
void set_handlers();
