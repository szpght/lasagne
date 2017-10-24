extern bad_syscall
extern syscall_count
extern syscall_table

thr_user_rsp            equ 0
thr_stack_top           equ 8
thr_syscall_type        equ 16
SYSCALL_BY_INTERRUPT    equ 0
SYSCALL_BY_SYSCALL      equ 1

global syscall_fast_handler
syscall_fast_handler:
    swapgs
    mov qword [gs:thr_user_rsp], rsp
    mov rsp, qword [gs:thr_stack_top]
    mov qword [gs:thr_syscall_type], SYSCALL_BY_SYSCALL
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
