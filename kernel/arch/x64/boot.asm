[BITS 32]
MAGIC         equ  0xE85250D6
ARCHITECTURE  equ  0
HEADER_LENGTH equ  16
CHECKSUM      equ  -(MAGIC + ARCHITECTURE + HEADER_LENGTH)

PAGE_PRESENT    equ 1 << 0
PAGE_RW         equ 1 << 1
PAGE_HUGE       equ 1 << 7

CR0_PAGING      equ 1 << 31
CRO_WRITE_PROT  equ 1 << 16

CR4_PAE         equ 1 << 5
CR4_PGE         equ 1 << 7
CR4_SMEP        equ 1 << 20

IA32_EFER       equ 0xC0000080
IA32_EFER_LME   equ 1 << 8
IA32_EFER_NXE   equ 1 << 11

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
resb 16384
stack_top:


section .bootstrap
align 4096
; reserve space for initial paging structures
global _pml4t
_pml4t:
    times 4096 db 0
pdptl:
    times 4096 db 0
pdpth:
    times 4096 db 0
pd:
    times 4096 db 0

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

; this function maps first 1 GiB of memory 
; at 0x0000000000000000 and 0xFFFF800000000000
global _start
_start:
    ; store multiboot2 information pointer
    mov edi, ebx

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
    
    ; setup level-4 table
    mov eax, _pml4t
    mov DWORD [eax], pdptl
    or DWORD [eax], PAGE_PRESENT | PAGE_RW
    
    ; move level-4 pointer to last chunk of higher half memory
    add eax, 8 * 511

    mov DWORD [eax], pdpth
    or DWORD [eax], PAGE_PRESENT | PAGE_RW

    ; set first entry in low PDPE to point at pd
    mov eax, pdptl
    mov DWORD [eax], pd
    add DWORD [eax], PAGE_PRESENT | PAGE_RW

    ; set second entry from the end in high PDPE to point at pd
    mov eax, pdpth
    add eax, 8 * 510
    mov DWORD [eax], pd
    add DWORD [eax], PAGE_PRESENT | PAGE_RW

    ; map first 1 GiB in PD because why not
    mov eax, pd
    mov ebx, PAGE_PRESENT | PAGE_RW | PAGE_HUGE
    mov ecx, 512
    .map_gib:
    mov [eax], ebx
    add eax, 8
    add ebx, 2 * 1024 * 1024
    dec ecx
    test ecx, ecx
    jnz .map_gib

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
    ; set final GDT
    lgdt [GDT64.pointer]
    mov ax, GDT64.data
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; enable global page support
    mov rax, cr4
    or rax, CR4_PGE
    mov cr4, rax

    mov rsp, stack_top
    mov rbp, rsp

    extern initialize
    mov rax, initialize
    call rax

    .halt:
    hlt
    jmp .halt


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
