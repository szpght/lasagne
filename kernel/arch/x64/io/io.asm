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

; rdi - register
global rdmsr
rdmsr:
    mov ecx, edi
    rdmsr
    shl rdx, 32
    or rax, rdx
    ret

; rdi - register
; rsi - data
global wrmsr
wrmsr:
    mov ecx, edi
    mov eax, esi
    mov rdx, rsi
    shr rdx, 32
    wrmsr
    ret
