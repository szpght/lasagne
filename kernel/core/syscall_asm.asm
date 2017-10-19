global syscall_fast_handler
syscall_fast_handler:    
    ; swapgs
    ; mov qword [gs:0], rsp
    ; mov rsp, qword [gs:8]

    ; push rcx
    ; push r11

    ; pop r11
    ; pop rcx
    
    ; mov rsp, qword [gs:0]
    ; swapgs

    ; xchg bx, bx
    sysret

    .hehe: db 'hehe', 10, 0
