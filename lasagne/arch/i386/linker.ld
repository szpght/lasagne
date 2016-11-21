ENTRY(_start)

SECTIONS
{
    . = 0x80000000;

    __vmem_base = .;

    . += 0x00100000;

    .text : AT(ADDR(.text) - 0x80000000)
    {
        __multiboot_header_start = . - 0x80000000;
        *(.multiboot)
        __text_start = . - 0x80000000;
        *(.text)
    }

    .rodata ALIGN(4K) : AT(ADDR(.rodata) - 0x80000000)
    {
        __rodata_start = . - 0x80000000;
        *(.rodata)
    }

    .data ALIGN(4K) : AT(ADDR(.data) - 0x80000000)
    {
        __data_start = . - 0x80000000;
        *(.data)
    }

    .bss ALIGN(4K) : AT(ADDR(.data) - 0x80000000)
    {
        __bss_start = . - 0x80000000;
        *(COMMON)
        *(.bss)
    }

    .page_directory ALIGN(4K) : AT(ADDR(.page_directory) - 0x80000000)
    {
        __page_directory = . - 0x80000000;
        . += 4K;
    }

    .page_tables ALIGN(4K) : AT(ADDR(.page_tables) - 0x80000000)
    {
        __kernel_page_tables = . - 0x80000000;
        . += 2M;
    }

    .nonkernel_memory ALIGN(4K) : AT(ADDR(.nonkernel_memory) - 0x80000000)
    {
        __free_memory_start = . - 0x80000000;
    }
}
