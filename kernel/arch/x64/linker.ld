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

    .text ALIGN(4K) : AT(ADDR(.text) - _KERNEL_VMA)
    {
        *(.text)
    }

    .eh_frame ALIGN(4K) : AT(ADDR(.eh_frame) - _KERNEL_VMA)
    {
        *(.eh_frame)
    }

    .rodata ALIGN(4K) : AT(ADDR(.rodata) - _KERNEL_VMA)
    {
        *(.rodata)
        *(.rodata.*)
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

    _KERNEL_END = ALIGN(4K);

    .comment ALIGN(4K) : AT(ADDR(.comment) - _KERNEL_VMA)
    {
        *(.comment)
    }

    .scratchpad ALIGN(4K) : AT(ADDR(.scratchpad) - _KERNEL_VMA)
    {
        . += 2M;
    }
}
