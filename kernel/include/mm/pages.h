#pragma once
#include <stdint.h>

#define PG_PRESENT (1L << 0)
#define PG_RW (1L << 1)
#define PG_USER (1L << 2)
#define PG_PWT (1L << 3)
#define PG_PCD (1L << 4)
#define PG_ACCESS (1L << 5)
#define PG_DIRTY (1L << 6)
#define PG_HUGE (1L << 7)
#define PG_PAT (1L << 7)
#define PG_GLOBAL (1L << 8)
#define PG_NX (1L << 63)
#define PG_HUGE_PAT (1L << 12)

// bits ignored in all tables
#define PG_BIT1 (1L << 9)
#define PG_BIT2 (1L << 10)
#define PG_BIT3 (1L << 11)
#define PG_BIT4 (1L << 61)
#define PG_BIT5 (1L << 62)

// bits ignored at all levels but lowest
#define PG_BIT6 (1L << 6)

// bits storing reference count
#define PG_REFCOUNT_OFFSET 52
// TODO use 9 bits
#define PG_REFCOUNT_MASK (0x3FFUL << 52)

// index in PML4 pointing to self
#define PG_SELF 510UL

// masks for page tables indices in virtual address
#define L1PTE_MASK (0x1FFUL << 12)
#define L2PTE_MASK (0x1FFUL << 21)
#define L3PTE_MASK (0x1FFUL << 30)
#define L4PTE_MASK (0x1FFUL << 39)

#define PHYS_ADDR_MASK 0xFFFFFFFFFF000

uintptr_t *pte(uintptr_t address, int level);

void reload_paging();
void map_page(uintptr_t virtual, uintptr_t physical, uint64_t flags);
void unmap_page(uintptr_t virtual, uint64_t flags);
