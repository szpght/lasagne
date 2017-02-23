[BITS 32]
MBALIGN     equ  1 << 0 ; align loaded modules on page boundaries
MEMINFO     equ  1 << 1 ; provide memory map
FLAGS       equ  MBALIGN | MEMINFO
MAGIC       equ  0x1BADB002
CHECKSUM    equ  -(MAGIC + FLAGS)

PAGE_PRESENT    equ 1 << 0
PAGE_RW         equ 1 << 1
PAGE_HUGE        equ 1 << 7

CR0_PAGING      equ 1 << 31

CR4_PAE         equ 1 << 5
CR4_PSE         equ 1 << 4

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM



section .bootstrap_data
align 4096
; reserve space for initial paging structures
pml4t:
    resb 4096
pdpt:
    resb 4096
pd:
    resb 4096

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


section .bootstrap
; this function maps first 1 GiB of memory 
; at 0x0000000000000000 and 0xFFFF800000000000
global _start
_start:
    ; set PML4T address
    mov eax, pml4t
    mov cr3, eax
    


    ; zeroing paging structures
    mov eax, pml4t
    mov ecx, 4096 * 3 / 4

    zero_tables:
    mov DWORD [eax], 0
    add eax, 4
    dec ecx
    test ecx, ecx
    jnz zero_tables

    ; setup level-4 table
    mov eax, pml4t
    mov DWORD [eax], pdpt
    or DWORD [eax], PAGE_PRESENT | PAGE_RW
    
    ; move level-4 pointer to beginning of higher half memory
    add eax, 8 * 256

    mov DWORD [eax], pdpt
    or DWORD [eax], PAGE_PRESENT | PAGE_RW

    ; set first entry in PDPE to point at pd
    mov eax, pdpt
    mov DWORD [eax], pd
    add DWORD [eax], PAGE_PRESENT | PAGE_RW

    ; map first 1 GiB in PD because why not
    mov eax, pd
    mov ebx, PAGE_PRESENT | PAGE_RW | PAGE_HUGE
    mov ecx, 512
    map_gib:
    mov [eax], ebx
    add eax, 8
    add ebx, 2 * 1024 * 1024
    dec ecx
    test ecx, ecx
    jnz map_gib

    xchg bx, bx

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

[BITS 64]
_start64:
    cli
    mov rdi, 0xFFFF8000000B8000
    mov WORD [rdi], 0x3c3c

    .halt:
    hlt
    jmp .halt