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


section .bootstrap
align 4096
; reserve space for initial paging structures
global _pml4t
_pml4t:
    times 4096 db 0
pdpt:
    times 4096 db 0
identity_pd:
%assign i (PAGE_PRESENT | PAGE_RW | PAGE_HUGE)
%rep 512
    dq i
%assign i (i + 2 * 1024 * 1024)
%endrep
kernel_pd:
    times 4096 db 0

temp_stack:
    times 32 db 0
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



    ; SETUP PAGING
    extern _KERNEL_TEXT_PHYS
    extern _KERNEL_EH_FRAME_PHYS
    extern _KERNEL_RODATA_PHYS
    extern _KERNEL_DATA_PHYS
    extern _KERNEL_BSS_PHYS
    extern _KERNEL_END_PHYS

    %define Kernel_end esi
    mov Kernel_end, _KERNEL_END_PHYS


    ; setup level-4 table
    ; point first and last entry to pdpt
    mov eax, _pml4t
    mov ebx, pdpt
    or ebx, PAGE_PRESENT | PAGE_RW
    mov DWORD [eax], ebx
    mov DWORD [eax + 8 * 511], ebx
    ; set refcount of pdpt to 1
    mov ebx, 1 << (REFCOUNT_OFFSET - 32)
    add DWORD [eax + 8 * 511 + 4], ebx

    ; point entry 510 to self
    mov ebx, eax
    or ebx, PAGE_PRESENT | PAGE_RW
    mov DWORD [eax + 8 * 510], ebx

    ; setup pdpt
    ; entry 0 points to identity pd
    mov eax, pdpt
    mov ebx, identity_pd
    or ebx, PAGE_PRESENT | PAGE_RW
    mov DWORD [eax], ebx
    
    ; entry 510 points to kernel pd
    mov ebx, kernel_pd
    or ebx, PAGE_PRESENT | PAGE_RW
    mov DWORD [eax + 8 * 510], ebx

    ; calculate number of page tables needed for kernel
    %define Kernel_pts ebp

    mov eax, _KERNEL_END_PHYS
    cdq ; convert double to quad
    mov ebx, 2 * 1024 * 1024
    div ebx
    ; add one page table if remainder present
    test edx, edx
    jz .round_number
    inc eax

    .round_number:
    mov Kernel_pts, eax

    ; set refcount of kernel pd in pdpt
    mov ebx, Kernel_pts
    shl ebx, REFCOUNT_OFFSET - 32
    add DWORD [pdpt + 8 * 510 + 4], ebx

    ; store count of bytes remaining in last pt in eax
    mov eax, edx
    ; calculate count of pt entries in last pt
    shr eax, 12
    ; calculate number to add in last pd
    shl eax, REFCOUNT_OFFSET - 32

    ; fill kernel pd
    mov ecx, Kernel_pts
    mov ebx, Kernel_end
    or ebx, PAGE_PRESENT | PAGE_RW
    mov edx, kernel_pd
    .fill_kernel_pd:
    mov DWORD [edx], ebx
    add ebx, 4096
    add edx, 8
    loop .fill_kernel_pd

    ; set refcount in last entry
    add DWORD [edx - 4], eax

    ; now the great moment of allocating and filling kernel page tables
    ; at this moment we have following variables:
    ; Kernel_end - pointing at the end of the kernel according to ld
    ; Kernel_pts - number of needed page tables

    ; registers that will be used
    ; eax - current page table entry address
    ; ecx - lower 32 bits of entry to be written
    ; ebx - upper 32 bits of entry to be written
    ; edx - beginning of next section

    mov eax, Kernel_end

    ; first 1 MiB of memory
    mov ebx, MAGIC_NX
    mov ecx, PAGE_PRESENT | PAGE_RW | PAGE_GLOBAL
    mov edx, _KERNEL_TEXT_PHYS
    call .map_pages

    ; .text section
    mov ecx, edx
    xor ebx, ebx
    or ecx, PAGE_PRESENT | PAGE_GLOBAL
    mov edx, _KERNEL_EH_FRAME_PHYS
    call .map_pages

    ; .eh_frame section
    mov ecx, edx
    mov ebx, MAGIC_NX
    or ecx, PAGE_PRESENT | PAGE_RW | PAGE_GLOBAL
    mov edx, _KERNEL_RODATA_PHYS
    call .map_pages

    ; .rodata section
    mov ecx, edx
    mov ebx, MAGIC_NX
    or ecx, PAGE_PRESENT | PAGE_GLOBAL
    mov edx, _KERNEL_DATA_PHYS
    call .map_pages

    ; .data section
    mov ecx, edx
    mov ebx, MAGIC_NX
    or ecx, PAGE_PRESENT | PAGE_RW | PAGE_GLOBAL
    mov edx, _KERNEL_BSS_PHYS
    call .map_pages

    ; .bss section
    mov ecx, edx
    mov ebx, MAGIC_NX
    or ecx, PAGE_PRESENT | PAGE_RW | PAGE_GLOBAL
    mov edx, _KERNEL_END_PHYS
    call .map_pages

    ; clean the rest of the pt
    mov ecx, 0
    ; calculate end address of page tables
    mov edx, Kernel_pts
    shl edx, 12
    add edx, Kernel_end
    ; do the cleaning
    .cleaning:
    mov DWORD [eax], 0
    mov DWORD [eax + 4], 0
    add eax, 8
    cmp eax, edx
    jne .cleaning


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

    .map_pages:
    mov DWORD [eax], ecx
    mov DWORD [eax + 4], ebx
    add eax, 8
    add ecx, 4096
    cmp edx, ecx
    jge .map_pages
    ret

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
