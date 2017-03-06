section .text

extern printk
global int_stub_handler
int_stub_handler:
    mov rdi, interrupt_msg
    call printk
    
    .forever:
    hlt
    jmp .forever


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


section .rodata

interrupt_msg:
    db "Unsupported interrupt occured", 0
