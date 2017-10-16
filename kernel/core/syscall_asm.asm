extern bad_syscall
extern syscall_count
extern syscall_table

global syscall_fast_handler
syscall_fast_handler:
    swapgs
    mov qword [gs:0], rsp
    mov rsp, qword [gs:8]
    swapgs

    push rcx
    push r11

    mov rcx, r10

    cmp rax, qword [syscall_count]
    jb .continue

    mov rdi, rax
    call bad_syscall
    jmp .exit

    .continue:
    shl rax, 3
    mov rax, qword [syscall_table + rax]
    call rax

    .exit:
    pop r11
    pop rcx
    swapgs
    mov rsp, qword [gs:0]
    swapgs

    o64 sysret
