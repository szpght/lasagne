#pragma once
#include <stdint.h>
#include <stddef.h>
#include <irq.h>

#define PG_SIZE 4096
#define PT_ENTRY_COUNT 512

// bits defined by amd64
#define PG_PRESENT (1ULL << 0)
#define PG_RW (1ULL << 1)
#define PG_USER (1ULL << 2)
#define PG_PWT (1ULL << 3)
#define PG_PCD (1ULL << 4)
#define PG_ACCESS (1ULL << 5)
#define PG_DIRTY (1ULL << 6)
#define PG_HUGE (1ULL << 7)
#define PG_PAT (1ULL << 7)
#define PG_GLOBAL (1ULL << 8)
#define PG_NX (1ULL << 63)
#define PG_HUGE_PAT (1ULL << 12)

// bits ignored in all tables
#define PG_BIT1 (1ULL << 9)
#define PG_BIT2 (1ULL << 10)
#define PG_BIT3 (1ULL << 11)
#define PG_BIT4 (1ULL << 61)
#define PG_BIT5 (1ULL << 62)

// bits ignored at all levels but lowest
#define PG_BIT6 (1ULL << 6)

// page fault error code flags
#define PF_PROT_VIOLATION (1ULL << 0)
#define PF_WRITE (1ULL << 1)
#define PF_USER (1ULL << 2)
#define PF_RSVD (1ULL << 3)
#define PF_INSTRUCTION_FETCH (1ULL << 4)

// kernel flags
#define MAP_IMMEDIATE (1ULL << 12)
#define MAP_LAZY PG_BIT1
#define MAP_RW PG_RW
#define MAP_USER PG_USER

// bits storing reference count
#define PG_REFCOUNT_OFFSET 52
// TODO use 9 bits
#define PG_REFCOUNT_MASK (0x3FFUL << 52)

// index in PML4 pointing to self
#define PG_SELF 510UL

// masks for page tables indices in virtual address
#define L1PTE_MASK (0x1FFULL << 12)
#define L2PTE_MASK (0x1FFULL << 21)
#define L3PTE_MASK (0x1FFULL << 30)
#define L4PTE_MASK (0x1FFULL << 39)

#define PHYS_ADDR_MASK 0xFFFFFFFFFF000

struct pt_entries {
    uintptr_t *entries[4];
    int present_mask;
};

void initialize_virtual_memory();
struct pt_entries ptes(uintptr_t address);
void reload_paging();
void map_page(uintptr_t virtual, uintptr_t physical, uint64_t flags);
uintptr_t unmap_page(uintptr_t virtual, uint64_t flags);
void map_range(uintptr_t start, size_t size, uint64_t flags);
void unmap_range(uintptr_t start, size_t size);
void zero_range(uintptr_t start, size_t size);
void page_fault_handler(struct irq_state *regs, uint64_t error_code);
