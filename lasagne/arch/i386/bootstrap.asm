MBALIGN  equ  1<<0 ; align loaded modules on page boundaries
MEMINFO  equ  1<<1 ; provide memory map
FLAGS    equ  MBALIGN | MEMINFO
MAGIC    equ  0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

section .multiboot
align 4
	dd MAGIC
	dd FLAGS
	dd CHECKSUM

section .bss
align 4
stack_bottom:
resb 16384
stack_top:

section .text
global _start
_start:
    ; virtual memory initalization
    extern __page_directory
    extern __kernel_page_tables
    extern __data_start
    extern __multiboot_header_start

    ; zeroing page directory's lower half
    xor ecx, ecx ; loop counter
    mov eax, __page_directory ; current pd entry pointer

    pd_zeroing:
    mov DWORD [eax], 0
    add eax, 4
    inc ecx
    cmp ecx, 512
    jne pd_zeroing

    ; setting pd upper half to proper page tables
    mov ebx, __kernel_page_tables ; current pt pointer with some attributes

    ; at first setting attributes
    add ebx, 3 ; 1 - present, 2 - rw

    pd_setting:
    mov DWORD [eax], ebx
    add eax, 4
    add ebx, 4096
    inc ecx
    cmp ecx, 1023
    jne pd_setting

    ; now zeroing page tables
    xor ecx, ecx
    mov eax, __kernel_page_tables

    pt_zeroing:
    mov DWORD [eax], 0
    inc ecx
    add eax, 4
    cmp ecx, 2*1024*1024/4
    jne pt_zeroing

    ; tables happily zeroed, now basic mapping
    ; set first 4MiB to identity paging
    mov DWORD [__page_directory], 0x00000083
    ; set last 4MiB to self
    mov DWORD [__page_directory + 4 * 1023], __page_directory + 3

    ; set pages for kernel structures
    ; at first first 1 MB, land where everithing is possible
    mov eax, 3 ; present, rw
    mov ebx, __kernel_page_tables
    mov ecx, 1 * 1024 * 1024 / 4096

    firstmb_fill_pt:
    mov DWORD [ebx], eax
    add eax, 4096
    add ebx, 4
    dec ecx
    test ecx, ecx
    jnz firstmb_fill_pt

    ; .text and .rodata are just after 1st MiB
    ; and are both read-only
    sub eax, 2 ; clean rw bit
    mov ecx, __data_start
    sub ecx, __multiboot_header_start

    ro_fill_pt:
    mov DWORD [ebx], eax
    add eax, 4096
    add ebx, 4
    sub ecx, 4096
    test ecx, ecx
    jnz ro_fill_pt

    ; .data, .bss
    add eax, 2 ; darkneeee^Wrw once again
    mov ecx, __page_directory
    sub ecx, __data_start

    rw_fill_pt:
    mov DWORD [ebx], eax
    add eax, 4096
    add ebx, 4
    sub ecx, 4096
    test ecx, ecx
    jnz rw_fill_pt

    ; set page directory address
    mov eax, __page_directory
    mov cr3, eax

    ; enable PSE
    mov eax, cr4
    or eax, 0x00000010
    mov cr4, eax

    mov eax, cr0
    ; set paging bit
    or eax, 0x80000000
    ; set write protect bit
    or eax, 0x10000
    mov cr0, eax

    lea eax, [.upper]
    jmp eax

.upper:
    ; unmap first page
    mov DWORD [__page_directory], 0
    invlpg [0]

	mov esp, stack_top

	extern kernel_main
	push ebx
	call kernel_main
    cli
.hang:	hlt
	jmp .hang
.end:
