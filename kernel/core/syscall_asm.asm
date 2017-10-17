extern user_stack
extern kernel_stack

global syscall_fast_handler
syscall_fast_handler:
    xchg bx, bx
    
    mov qword [user_stack], rsp
    mov rsp, qword[kernel_stack]
    mov eax, 3
    int 0x30
    push rcx
    push r11
    mov rdi, .hehe
    extern printk
    call printk
    xchg bx, bx
    pop r11
    pop rcx
    sysret

    .hehe: db 'hehe', 10, 0