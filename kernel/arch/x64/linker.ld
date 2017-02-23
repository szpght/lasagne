ENTRY(_start)
_KERNEL_LMA = 0x00100000;
_KERNEL_VMA = 0xFFFFFFFF80000000;

SECTIONS
{
    .  = 0;

    .multiboot :
    {
        *(.multiboot)
    }

    . = _KERNEL_LMA;

    .bootstrap :
    {
        *(.bootstrap)
        *(.bootstrap_data)
    }

    . += _KERNEL_VMA;

    .text : AT(ADDR(.text) - _KERNEL_VMA)
    {
        *(.text)
    }

    .rodata ALIGN(4K) : AT(ADDR(.rodata) - _KERNEL_VMA)
    {
        *(.rodata)
    }

    .data ALIGN(4K) : AT(ADDR(.data) - _KERNEL_VMA)
    {
        *(.data)
    }

    .bss ALIGN(4K) : AT(ADDR(.bss) - _KERNEL_VMA)
    {
        *(COMMON)
        *(.bss)
    }
}
