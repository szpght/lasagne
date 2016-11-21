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
global _start:function (_start.end - _start)
_start:
	mov esp, stack_top

	extern kernel_main
	push ebx
	call kernel_main
    cli
.hang:	hlt
	jmp .hang
.end:
