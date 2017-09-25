#include <mm/pages.h>

#define __boot __attribute__ ((section(".boot")))

extern uint64_t _pml4t[];
extern uint64_t pdpt[];
extern uint64_t kernel_pd[];
extern uint64_t identity_pd[];

extern uint64_t _KERNEL_TEXT_PHYS[];
extern uint64_t _KERNEL_EH_FRAME_PHYS[];
extern uint64_t _KERNEL_RODATA_PHYS[];
extern uint64_t _KERNEL_DATA_PHYS[];
extern uint64_t _KERNEL_BSS_PHYS[];
extern uint64_t _KERNEL_END_PHYS[];

extern uint64_t *__kernel_end_phys;

__boot void initialize_paging()
{
    const uint64_t RW = PG_PRESENT | PG_RW;
    const uint32_t KERNEL_END = (uint32_t) _KERNEL_END_PHYS;
    // level 4 table setup
    // point first and lat entry to pdpt
    // set refcount to 1
    uint64_t entry = (uint32_t) pdpt | RW | (1ULL << PG_REFCOUNT_OFFSET);

    _pml4t[0] = entry;
    _pml4t[511] = entry;

    // point entry #PG_SELF to self
    _pml4t[PG_SELF] = (uint32_t) _pml4t | RW;

    // level 3 table setup
    pdpt[0] = (uint32_t) identity_pd | RW;
    pdpt[510] = (uint32_t) kernel_pd | RW;

    // set refcount of kernel page directory to number of pts used by kernel
    const int pt_range = PT_ENTRY_COUNT * PG_SIZE;
    const int full_kernel_pages = KERNEL_END / pt_range;
    const int bytes_remaining_in_last_pt = KERNEL_END % pt_range;
    const int page_tables_for_kernel = bytes_remaining_in_last_pt ?
        full_kernel_pages + 1 : full_kernel_pages;

    pdpt[510] |= (uint64_t) page_tables_for_kernel << PG_REFCOUNT_OFFSET;

    // fill kernel pd
    for (int i = 0; i < page_tables_for_kernel; ++i) {
        kernel_pd[i] = (KERNEL_END + i * PG_SIZE) | RW;
    }

    const uint64_t entries_in_last_pt = bytes_remaining_in_last_pt / PG_SIZE;
    kernel_pd[page_tables_for_kernel - 1] |= entries_in_last_pt << PG_REFCOUNT_OFFSET;

    // fill kernel pts
    uint64_t *current_pte = _KERNEL_END_PHYS;
    uint64_t current_page = 0;
    uint64_t flags = 0;

    // first 1 MiB of memory
    flags = RW | PG_GLOBAL | PG_NX;
    while (current_page < (uint32_t) _KERNEL_TEXT_PHYS) {
        *current_pte = current_page | flags;
        current_pte += 1;
        current_page += PG_SIZE;
    }

    // text section
    flags = PG_PRESENT | PG_GLOBAL;
    while (current_page < (uint32_t) _KERNEL_EH_FRAME_PHYS) {
        *current_pte = current_page | flags;
        current_pte += 1;
        current_page += PG_SIZE;
    }

    // eh_frame section
    flags = RW | PG_GLOBAL | PG_NX;
    while (current_page < (uint32_t) _KERNEL_RODATA_PHYS) {
        *current_pte = current_page | flags;
        current_pte += 1;
        current_page += PG_SIZE;
    }

    // rodata section
    flags = PG_PRESENT | PG_GLOBAL | PG_NX;
    while (current_page < (uint32_t) _KERNEL_DATA_PHYS) {
        *current_pte = current_page | flags;
        current_pte += 1;
        current_page += PG_SIZE;
    }

    // data and bss sections
    flags = RW | PG_GLOBAL | PG_NX;
    while (current_page < (uint32_t) _KERNEL_BSS_PHYS) {
        *current_pte = current_page | flags;
        current_pte += 1;
        current_page += PG_SIZE;
    }

    // bss section
    flags = RW | PG_GLOBAL | PG_NX;
    while (current_page < (uint32_t) _KERNEL_END_PHYS) {
        *current_pte = current_page | flags;
        current_pte += 1;
        current_page += PG_SIZE;
    }

    // clean the rest of the last pt
    uint64_t *end_of_page_tables =
        (uint64_t *) (page_tables_for_kernel * PG_SIZE + KERNEL_END);

    while (current_pte < end_of_page_tables) {
        *current_pte = 0;
        current_pte += 1;
    }

    __kernel_end_phys = current_pte;
}
