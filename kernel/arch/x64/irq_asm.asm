section .text

extern printk
global int_stub_handler
int_stub_handler:
    mov rdi, interrupt_msg
    call printk
    jmp sleep


global page_fault_handler
page_fault_handler:
    mov rdi, page_fault_msg
    mov rsi, cr2
    pop rdx
    call printk
    jmp sleep


global enable_irq
enable_irq:
    sti
    ret


global disable_irq
disable_irq:
    cli
    ret


global _load_idt
_load_idt:
    lidt [rdi]
    ret


sleep:
    hlt
    jmp sleep


section .rodata

interrupt_msg:
    db "Unsupported interrupt occured", 0
page_fault_msg:
    db "Page fault at address %lx, error code %x", 0
