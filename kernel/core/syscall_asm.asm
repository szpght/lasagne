extern user_stack
extern kernel_stack

global syscall_fast_handler
syscall_fast_handler:
    xchg bx, bx
    
    mov qword [gs:0], rsp
    swapgs
    mov rsp, qword [gs:0]

    push rcx
    push r11

    push rax
    pop rax    
    mov eax, 3
    int 0x30

    mov rdi, .hehe
    extern printk
    call printk
    xchg bx, bx
    
    pop r11
    pop rcx
    
    swapgs
    mov rsp, qword [gs:0]

    sysret

    .hehe: db 'hehe', 10, 0