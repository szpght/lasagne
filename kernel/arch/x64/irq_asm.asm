section .text

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


align 8, nop
global interrupt_wrapper
interrupt_wrapper:
    %assign i 0
    ; this number must be consistent with INT_VECTORS_NUMBER in irq.h
    %rep 80
    push i
    jmp handler
    align 8, nop
    %assign i i+1
    %endrep


SIZEOF_IDT_HANDLER equ 16
INT_HANDLER_ERRORCODE equ 1 << 0
INT_HANDLER_RETVAL equ 1 << 1
extern idt_handler
handler:
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax

    ; callee-saved registers:
    ; rbx, rbp, r12-r16

    ; get interrupt offset in table
    mov rbx, [rsp + 8 * 15]
    imul rbx, SIZEOF_IDT_HANDLER

    ; get register state
    mov rdi, rsp

    ; get error code
    mov rsi, [rsp + 8 * 16]

    call [idt_handler + rbx]

    ; check if interrupt returns value
    test QWORD [idt_handler + rbx + 8], INT_HANDLER_RETVAL
    jz .doesnt_return_value
    mov QWORD [rsp], rax

    .doesnt_return_value:
    ; check if interrupt pushes error code
    test QWORD [idt_handler + rbx + 8], INT_HANDLER_ERRORCODE

    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop rbp
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15

    ; pop error code if present
    jz .no_error_code
    add rsp, 8

    .no_error_code:
    ; pop interrupt number
    add rsp, 8

    iretq


global get_cr2
get_cr2:
mov rax, cr2
ret