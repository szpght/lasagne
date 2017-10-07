global load_tss
load_tss:
    mov rax, 0x28
    ltr ax
    ret

global switch_task_sys
switch_task_sys:
    ; save state to current stack
    pushfq
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

    ; save rsp
    mov QWORD [rdi], rsp

    ; restore rsp of new task
    mov rsp, QWORD rsi

    ; restore state from new stack
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
    popfq
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
