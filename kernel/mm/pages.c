#include <mm/pages.h>
#include <mm/frame.h>
#include <assert.h>

void reload_paging()
{
    __asm__ volatile ("mov %cr3, %rax");
    __asm__ volatile ("mov %rax, %cr3");
}

static void invlpg(uintptr_t address)
{
    __asm__ volatile ("invlpg (%0)" : : "b"(address) : "memory");
}

uintptr_t *pte(uintptr_t address, int level)
{
    assert(level >= 0 && level <= 4);

    for (int i = 0; i < level; ++i) {
        address >>= 9;
        address &= ~L4PTE_MASK;
        address |= PG_SELF << 39;
    }
    address = (address & ~7UL) | 0xFFFF000000000000;
    return (uintptr_t*) address;
}

void pt_gc_acquire(uintptr_t address, int level)
{
    if (level > 3) {
        return;
    }
    // increase counter only if entry empty
    if (*pte(address, level) & PG_PRESENT) {
        return;
    }

    uintptr_t *entry = pte(address, level + 1);
    // TODO use 9 bits
    *entry += 1L << PG_REFCOUNT_OFFSET;
}

void pt_gc_release(uintptr_t address, int level)
{
    if (level > 3) {
        return;
    }
    uintptr_t *entry = pte(address, level + 1);
    *entry -= 1L << PG_REFCOUNT_OFFSET;
    if ((*entry & PG_REFCOUNT_MASK) == 0) {
        free_frame(*entry & PHYS_ADDR_MASK);
        *entry = 0;
        pt_gc_release(address, level + 1);
    }
}

void create_pts(uintptr_t address, int level)
{
    while (level > 1) {
        uintptr_t *entry = pte(address, level);
        pt_gc_acquire(address, level);
        *entry = get_frame() | PG_PRESENT | PG_RW;
        --level;
        // TODO zeroing tables
    }
}

void ensure_pt_exists(uintptr_t address)
{
    for (int level = 4; level > 1; --level) {
        uintptr_t *entry = pte(address, level);
        if (!(*entry & PG_PRESENT)) {
            create_pts(address, level);
            return;
        }
    }
}

void map_page(uintptr_t virtual, uintptr_t physical, uint64_t flags)
{
    assert(virtual % 4096 == 0);
    assert(physical % 4096 == 0);
    ensure_pt_exists(virtual);
    uintptr_t *entry = pte(virtual, 1);
    pt_gc_acquire(virtual, 1);
    *entry = physical | flags;
    invlpg(virtual);
}

void unmap_page(uintptr_t virtual, uint64_t flags)
{
    uintptr_t *entry = pte(virtual, 1);
    *entry = flags;
    pt_gc_release(virtual, 1);
    invlpg(virtual);
}
