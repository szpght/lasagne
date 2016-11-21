extern void *__vmem_base;
extern void *__multiboot_header_start;
extern void *__text_start;
extern void *__rodata_start;
extern void *__data_start;
extern void *__bss_start;
extern void *__page_directory;
extern void *__kernel_page_tables;
extern void *__free_memory_start;

#define vmem_base ((void*)&__vmem_base)
#define multiboot_header ((void*)&__multiboot_header_start)
#define text_section ((void*)&__text_start)
#define rodata_section ((void*)&__rodata_start)
#define data_section ((void*)&__data_start)
#define bss_section ((void*)&__bss_start)
#define page_directory ((void*)&__page_directory)
#define kernel_page_tables ((void*)&__kernel_page_tables)
#define free_memory_start ((void*)&__free_memory_start)
