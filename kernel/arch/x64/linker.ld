ENTRY(_start)
_KERNEL_LMA = 0x00100000;
_KERNEL_VMA = 0xFFFFFFFF80000000;

SECTIONS
{
    . = _KERNEL_LMA;

    .boot ALIGN(4K) :
    {
        *(.multiboot)
        *(.boot)
        *(.boot.data)
        *(.comment)
    }

    .data_low ALIGN(4K) :
    {
        *(.data_low)
    }

    . += _KERNEL_VMA;

    .text ALIGN(4K) : AT(ADDR(.text) - _KERNEL_VMA)
    {
        _KERNEL_TEXT_PHYS = . - _KERNEL_VMA;
        _KERNEL_TEXT = .;
        *(.text)
    }

    .rodata ALIGN(4K) : AT(ADDR(.rodata) - _KERNEL_VMA)
    {
        _KERNEL_RODATA_PHYS = . - _KERNEL_VMA;
        _KERNEL_RODATA = .;
        *(.rodata)
        *(.rodata.*)
    }

    .data ALIGN(4K) : AT(ADDR(.data) - _KERNEL_VMA)
    {
        _KERNEL_DATA_PHYS = . - _KERNEL_VMA;
        _KERNEL_DATA = .;
        *(.data)
    }

    .bss ALIGN(4K) : AT(ADDR(.bss) - _KERNEL_VMA)
    {
        _KERNEL_BSS_PHYS = . - _KERNEL_VMA;
        _KERNEL_BSS = .;
        *(COMMON)
        *(.bss)
    }

    _KERNEL_END = ALIGN(4K);
    _KERNEL_END_PHYS = ALIGN(4K) - _KERNEL_VMA;

    .scratchpad ALIGN(4K) : AT(ADDR(.scratchpad) - _KERNEL_VMA)
    {
        . += 2M;
    }
}
