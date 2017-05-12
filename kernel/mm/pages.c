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

struct pt_entries ptes(uintptr_t address)
{
    struct pt_entries e;
    e.present_mask = 0;

    for (int i = 0; i < 4; ++i) {
        address >>= 9;
        address &= ~(L4PTE_MASK | 7UL);
        address |= 0xFFFF000000000000 | (PG_SELF << 39);
        e.entries[3 - i] = address;
    }

    for (int i = 0; i < 4; ++i) {
        if (e.entries[i][0] & PG_PRESENT) {
            e.present_mask |= 1 << i;
        }
        else {
            return e;
        }
    }
    return e;
}

static void increase_counter(uintptr_t *pte)
{
    *pte += 1L << PG_REFCOUNT_OFFSET;
}

static int decrease_counter(uintptr_t *pte)
{
    *pte -= 1L << PG_REFCOUNT_OFFSET;
    return (*pte & PG_REFCOUNT_MASK) >> PG_REFCOUNT_OFFSET;
}

void ensure_pt_exists(struct pt_entries *entries)
{
    for (int i = 0; i < 3; ++i) {
        if (entries->present_mask & 1 << i) {
            continue;
        }

        *entries->entries[i] = get_frame() | PG_PRESENT | PG_RW;
        if (i > 0 && i < 3) {
            increase_counter(entries->entries[i - 1]);
        }
    }
}

void clean_pts(struct pt_entries *entries)
{
    for (int i = 2; i >= 0; --i) {
        int count = decrease_counter(entries->entries[i]);
        if (count) {
            break;
        }
        free_frame(*entries->entries[i] & PHYS_ADDR_MASK);
        *entries->entries[i] = 0;
    }
}

void map_page(uintptr_t virtual, uintptr_t physical, uint64_t flags)
{
    assert(virtual % 4096 == 0);
    assert(physical % 4096 == 0);
    struct pt_entries e = ptes(virtual);
    if (!(e.present_mask & 1 << 2)) {
        ensure_pt_exists(&e);
    }
    if (!(e.present_mask & 1 << 3)) {
        increase_counter(e.entries[2]);
    }
    *e.entries[3] = physical | flags;
    invlpg(virtual);
}

uintptr_t unmap_page(uintptr_t virtual, uint64_t flags)
{
    assert(virtual % 4096 == 0);
    struct pt_entries e = ptes(virtual);
    uintptr_t physical = *e.entries[3] & PHYS_ADDR_MASK;
    e.entries[3] = 0;
    clean_pts(&e);
    return physical;
}
