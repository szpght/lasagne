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



void initialize_irq();
void initialize_pic();
void compile_idt(struct idt_entry *dest, struct idt_model *src);
void load_idt(uint16_t size, void *idt);
void create_idt();
void enable_irq();
void disable_irq();
void _load_idt(struct idtr *idtr);

// irq handlers, not to be called from C!
void int_stub_handler();
void page_fault_handler();
