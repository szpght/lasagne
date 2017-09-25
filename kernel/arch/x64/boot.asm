[BITS 32]
MAGIC         equ  0xE85250D6
ARCHITECTURE  equ  0
HEADER_LENGTH equ  16
CHECKSUM      equ  -(MAGIC + ARCHITECTURE + HEADER_LENGTH)

PAGE_PRESENT    equ 1 << 0
PAGE_RW         equ 1 << 1
PAGE_HUGE       equ 1 << 7
PAGE_GLOBAL     equ 1 << 8
PAGE_NX         equ 1 << 63
MAGIC_NX        equ 1 << 31


CR0_PAGING      equ 1 << 31
CRO_WRITE_PROT  equ 1 << 16

CR4_PAE         equ 1 << 5
CR4_PGE         equ 1 << 7
CR4_SMEP        equ 1 << 20

IA32_EFER       equ 0xC0000080
IA32_EFER_LME   equ 1 << 8
IA32_EFER_NXE   equ 1 << 11

REFCOUNT_OFFSET equ 52

extern _KERNEL_VMA


section .multiboot
align 4
    dd MAGIC
    dd ARCHITECTURE
    dd HEADER_LENGTH
    dd CHECKSUM


section .bss
align 4
stack_bottom:
resb 8192
stack_top:


section .data_low
align 4096

global _pml4t
_pml4t:
    times 4096 db 0
global pdpt
pdpt:
    times 4096 db 0
global kernel_pd
kernel_pd:
    times 4096 db 0

global __kernel_end_phys
__kernel_end_phys:
    dd 0

section .boot
align 4096
; reserve space for initial paging structures
global identity_pd
identity_pd:
%assign i (PAGE_PRESENT | PAGE_RW | PAGE_HUGE)
%rep 512
    dq i
%assign i (i + 2 * 1024 * 1024)
%endrep

temp_stack:
    times 512 db 0
temp_stack_end:

; temporary GDT
GDT:
    .null: equ $ - GDT           ; The null descriptor.
    dq 0
    .code: equ $ - GDT           ; The code descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 10011010b                 ; Access (exec/read).
    db 00100000b                 ; Granularity.
    db 0                         ; Base (high).
    .data: equ $ - GDT           ; The data descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 10010010b                 ; Access (read/write).
    db 00000000b                 ; Granularity.
    db 0                         ; Base (high).
    .pointer:
    dw $ - GDT - 1
    .base_pointer:
    dq GDT


global _start
_start:
    ; store multiboot2 information pointer
    mov edi, ebx

    ; setup temporary stack
    mov esp, temp_stack_end

    ; check if running on amd64

    ; check if CPUID supported
    ; flip ID bit and check if it worked
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 1 << 21
    push eax
    popfd
    pushfd
    pop eax
    cmp eax, ecx
    je .no_amd64

    ; check if testing for long mode available
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_amd64

    ; check if long mode actually supported
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz .no_amd64

    ; end of amd64 check

    ; set _pml4t address
    mov eax, _pml4t
    mov cr3, eax

    extern initialize_paging
    call initialize_paging

    ; enable PAE
    mov eax, cr4
    or eax, CR4_PAE
    mov cr4, eax

    ; set long mode bit in EFER MSR register
    mov ecx, IA32_EFER
    rdmsr
    or eax, IA32_EFER_LME | IA32_EFER_NXE
    wrmsr

    ; enable paging
    mov eax, cr0
    or eax, CR0_PAGING | CRO_WRITE_PROT
    mov cr0, eax

    lgdt [GDT.pointer]
    jmp GDT.code:_start64


    ; FUNCTIONS SECTION

    .no_amd64:
    mov ecx, 80 * 25
    mov eax, 0xB8000

    .clear_screen:
    mov WORD [eax], 0
    add eax, 2
    loop .clear_screen

    ; print message
    mov eax, 0xB8000
    mov bh, 0x0F ; white text on black background
    mov ecx, .message_end - .message
    mov edx, .message

    .print:
    mov bl, BYTE [edx]
    mov WORD [eax], bx
    add eax, 2
    inc edx
    loop .print

    .halt:
    hlt
    jmp .halt

    .message:
    db "64 bit mode not supported"
    .message_end:


[BITS 64]
_start64:
    jmp _starthigh64


section .text
_starthigh64:
    mov rsp, stack_top
    mov rbp, rsp

    ; set kernel end
    mov eax, DWORD [__kernel_end_phys]
    extern KERNEL_END
    mov QWORD [KERNEL_END], rax

    ; set final GDT
    call reload_gdt

    ; enable global page support
    mov rax, cr4
    or rax, CR4_PGE
    mov cr4, rax

    extern initialize
    mov rax, initialize
    call rax

    .halt:
    hlt
    jmp .halt

global reload_gdt
reload_gdt:
    lgdt [GDT64.pointer]
    mov ax, GDT64.data
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret


section .data
GDT64:                           ; Global Descriptor Table (64-bit).
    .null: equ $ - GDT64         ; The null descriptor.
    dq 0
    .code: equ $ - GDT64         ; The code descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 10011010b                 ; Access (exec/read).
    db 00100000b                 ; Granularity.
    db 0                         ; Base (high).
    .data: equ $ - GDT64         ; The data descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 10010010b                 ; Access (read/write).
    db 00000000b                 ; Granularity.
    db 0                         ; Base (high).
    .usercode: equ $ - GDT64     ; The code descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 11111010b                 ; Access (exec/read).
    db 00100000b                 ; Granularity.
    db 0                         ; Base (high).
    .userdata: equ $ - GDT64     ; The data descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 11110010b                 ; Access (read/write).
    db 00000000b                 ; Granularity.
    db 0                         ; Base (high).
    .tss: equ $ - GDT64
    times 16 db 0 ; will be filled at runtime

    .pointer:
    dw $ - GDT64 - 1
    dq GDT64

global tss_descriptor
tss_descriptor equ GDT64 + GDT64.tss
