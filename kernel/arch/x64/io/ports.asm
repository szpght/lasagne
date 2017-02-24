; rdi - port
; rsi - data
global outb
outb:
    mov rax, rsi
    mov rdx, rdi
    out dx, al
    ret

; rdi - port
global inb
inb:
    mov rdx, rdi
    xor rax, rax
    in al, dx
    ret