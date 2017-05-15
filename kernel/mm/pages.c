#include <mm/pages.h>
#include <mm/frame.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

static uintptr_t zero_page;

void initialize_virtual_memory()
{
    zero_page = get_frame();
    uint64_t *zero_page_v = 0xFFFFFFFF80001000;
    // TODO more structured approach to virtual memory
    uintptr_t old_addr = unmap_page(zero_page_v, 0);
    map_page(zero_page_v, zero_page, PG_PRESENT | PG_RW);
    memset(zero_page_v, 0, 4096);
    map_page(zero_page_v, old_addr, PG_PRESENT | PG_RW);
    set_irq_handler(14, page_fault_handler, INT_HANDLER_ERRORCODE);
}

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

static void ensure_pt_exists(struct pt_entries *entries)
{
    for (int i = 0; i < 3; ++i) {
        if (entries->present_mask & 1 << i) {
            continue;
        }

        *entries->entries[i] = get_frame() | PG_PRESENT | PG_RW;
        // TODO get zeroed page from frame allocator
        // zeroing newly created pt
        uintptr_t *new_pt = *entries->entries[i + 1] & ~0xFFFL;
        invlpg(new_pt);
        memset(new_pt, 0, 4096);
        if (i > 0) {
            increase_counter(entries->entries[i - 1]);
        }
    }
}

static void clean_pts(struct pt_entries *entries)
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

void map_range(uintptr_t start, size_t size, uint64_t flags)
{
    assert(!!(flags & MAP_LAZY) ^ !!(flags & MAP_IMMEDIATE));
    uintptr_t physical = zero_page;
    uint64_t map_flags = PG_PRESENT;
    map_flags |= flags & PG_USER;

    if (flags & MAP_LAZY) {
        map_flags |= MAP_LAZY;
    }
    else {
        map_flags |= flags & PG_RW;
    }
    for (size_t i = 0; i < size; i += 4096) {
        if (flags & MAP_IMMEDIATE) {
            printk("getting physical\n");
            physical = get_frame();
        }
        map_page(start + i, physical, map_flags);
    }
}

void unmap_range(uintptr_t start, size_t size)
{
    for (size_t i = 0; i < size; i += 4096) {
        struct pt_entries e = ptes(start + i);
        if (e.present_mask == 0xF) {
            bool frame_allocated = true;
            if (*e.entries[3] & MAP_LAZY) {
                frame_allocated = false;
            }
            uintptr_t physical = unmap_page(start + i, 0);
            if (frame_allocated) {
                free_frame(physical);
            }
        }
    }
}

void zero_range(uintptr_t start, size_t size)
{
    for (size_t i = 0; i < size; i += 4096) {
        struct pt_entries e = ptes(start + i);
        assert(e.present_mask == 0xF);
        if (e.present_mask == 0xF && !(*e.entries[3] & MAP_LAZY)) {
            uintptr_t physical = *e.entries[3] & PHYS_ADDR_MASK;
            free_frame(physical);
            *e.entries[3] &= ~PHYS_ADDR_MASK;
            *e.entries[3] |= zero_page | MAP_LAZY;
        }
    }
}

void page_fault_handler(struct irq_state *regs, uint64_t error_code)
{
    uintptr_t addr = get_cr2();
    struct pt_entries e = ptes(addr);

    // check if fault because of write to lazy page
    if ((error_code & PF_PROT_VIOLATION)
        && (error_code & PF_WRITE)
        && *e.entries[3] & MAP_LAZY) {

        uintptr_t flags = *e.entries[3] & ~(PHYS_ADDR_MASK | MAP_LAZY);
        flags |= PG_RW;
        *e.entries[3] = get_frame() | flags;
        invlpg(addr);
        return;
    }
    generic_exception_handler(regs, error_code);
}
