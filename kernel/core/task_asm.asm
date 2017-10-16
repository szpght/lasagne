global load_tss
load_tss:
    mov rax, 0x28
    ltr ax
    ret

global switch_task_int
switch_task_int:
    ; save rbp in case when next function on the call stack
    ; would restore rsp from rbp
    push rbp

    ; save rsp
    mov QWORD [rdi], rsp

    ; restore rsp of new task
    mov rsp, QWORD rsi

    pop rbp
    ret

global switch_task_sys
switch_task_sys:
    push rbx
    push r12
    push r13
    push r14
    push r15

    call switch_task_int

    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    xor eax, eax
    xor ecx, ecx
    xor edx, edx
    xor esi, esi
    xor r8, r8
    xor r9, r9
    xor r10, r10
    xor r11, r11
    ret

global usermode_function
usermode_function:
    mov rax, 1
    mov rdi, .text_to_print
    syscall

    mov rax, 2
    syscall
    
    mov r12, 0x1000;
    .over:
    mov rdi, .text2
    mov rsi, r12
    mov rax, 1
    syscall

    xor rax, rax
    syscall
    inc r12
    jmp .over
    .text_to_print:
    db 'Hello from user mode', 10, 0
    .text2:
    db 'counter %d', 10, 0

global idle_thread
idle_thread:
    hlt
    jmp idle_thread
