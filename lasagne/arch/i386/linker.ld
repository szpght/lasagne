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

    .kernel_structures ALIGN(4K) : AT(ADDR(.kernel_structures) - 0x80000000)
    {
        __page_directory = . - 0x80000000;
        . += 4K;
        __kernel_page_tables = . - 0x80000000;
        . += 2M;
        __frame_bitmap = . - 0x80000000;
        . += 128K;
        __free_memory_start = . - 0x80000000;
    }
}
