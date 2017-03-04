#pragma once
#include <stdint.h>

#define MB_MEMORY_MAP 6

#define MB_RAM_AVAILABLE 1
#define MB_RAM_ACPI 3
#define MB_RAM_HIBERNATION_PRESERVED 4


struct mb_header {
    uint32_t total_size;
    uint32_t reserved;
} __attribute__((packed));


struct mb_tag {
    uint32_t type;
    uint32_t size;
} __attribute__((packed));

struct mb_memmap {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
} __attribute__((packed));

struct mb_memmap_entry {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
} __attribute__((packed));


void parse_multiboot(void *mb);
void initialize_memory(void *mb);
void *physical_end(void *mb);
void mark_free(void *mb);
