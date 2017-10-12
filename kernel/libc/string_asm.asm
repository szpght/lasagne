global memset
memset:
	cld
	mov rcx, rdx
    mov rdx, rdi ; store dest pointer for return
	mov eax, esi
	rep stosb

    mov rax, rdx
	ret

global memcpy
memcpy:
    cld
    mov rcx, rdx
    mov rax, rdi ; return dest pointer
    rep movsb

    ret

global memmove
memmove:
    ; compute difference between pointers
    mov rax, rdi
    sub rax, rsi

    ; if dest below src, copy forward
    jb memcpy

    mov r8, rdi ; store dest for return
    je .return

    ; if dest above src, check if areas overlap
    cmp rax, rdx
    jae memcpy
    jb .backward

    .backward:
    std
    mov rcx, rdx
    add rdi, rcx
    dec rdi
    add rsi, rcx
    dec rsi
    rep movsb

    .return:
    mov rax, r8
    ret
