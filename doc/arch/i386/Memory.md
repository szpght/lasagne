Memory management
=================

Lasagne is a higher-half kernel. It is loaded at 1MiB offset. Physical memory from beginning to end of the kernel is mapped at 2GiB offset.

Kernel page directory and 2MiB of page tables are statically allocated by linker. They can be accessed by obtaining `__page_directory` and `__kernel_page_tables` symbols addresses, or by using constants `page_directory` and `kernel_page_tables` defined in `memory_sections.h`. There is also `__vmem_base` symbol and `vmem_base` constant that can be used to translate between physical and virtual addresses.

At boot, sections are given proper read/write permissions.

Memory map
----------

| Section               | Symbol                     | Constant             | Size     |
|-----------------------|----------------------------|----------------------|----------|
| Lower memory          | -                          | -                    | 1 MiB    |
| Multiboot header      | `__multiboot_header_start` | `multiboot_header`   | 12 B     |
| Text section          | `__text_start`             | `text_section`       | variable |
| Read-only data        | `__rodata_start`           | `rodata_section`     | variable |
| Data                  | `__data_start`             | `data_section`       | variable |
| BSS                   | `__bss_start`              | `bss_section`        | variable |
| Kernel page directory | `__page_directory`         | `page_directory`     | 4 KiB    |
| Kernel page tables    | `__kernel_page_tables`     | `kernel_page_tables` | 2 MiB    |