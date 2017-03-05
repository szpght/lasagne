[BITS 32]
MAGIC         equ  0xE85250D6
ARCHITECTURE  equ  0
HEADER_LENGTH equ  16
CHECKSUM      equ  -(MAGIC + ARCHITECTURE + HEADER_LENGTH)

PAGE_PRESENT    equ 1 << 0
PAGE_RW         equ 1 << 1
PAGE_HUGE        equ 1 << 7

CR0_PAGING      equ 1 << 31

CR4_PAE         equ 1 << 5
CR4_PSE         equ 1 << 4

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

; set GDT
GDT64:                           ; Global Descriptor Table (64-bit).
    .null: equ $ - GDT64         ; The null descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 0                         ; Access.
    db 0                         ; Granularity.
    db 0                         ; Base (high).
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
    .pointer:                    ; The GDT-pointer.
    dw $ - GDT64 - 1             ; Limit.
    dq GDT64                     ; Base.


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
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; enable paging
    mov eax, cr0
    or eax, CR0_PAGING
    mov cr0, eax

    lgdt [GDT64.pointer]
    jmp GDT64.code:_start64


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
    ; jump to high memory
    extern _KERNEL_VMA
    mov rax, .high_memory
    add rax, _KERNEL_VMA
    jmp rax

    .high_memory:
    mov rsp, stack_top
    mov rbp, rsp
    mov ax, GDT64.data
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    extern initialize
    mov rax, initialize
    call rax

    .halt:
    cli
    hlt
    jmp .halt