ENTRY(_start)
KERNEL_LMA = 0x00100000;
KERNEL_VMA = 0xFFFF800000000000;

SECTIONS
{
    .  = 0;

    .multiboot :
    {
        *(.multiboot)
    }

    . = KERNEL_LMA;

    .bootstrap :
    {
        *(.bootstrap)
        *(.bootstrap_data)
    }

    . += KERNEL_VMA;

    .text : AT(ADDR(.text) - KERNEL_VMA)
    {
        *(.text)
    }

    .rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_VMA)
    {
        *(.rodata)
    }

    .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VMA)
    {
        *(.data)
    }

    .bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_VMA)
    {
        *(COMMON)
        *(.bss)
    }
}
