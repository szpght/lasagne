#include <mm/frame.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <mm/memory_map.h>
#include <printk.h>

#define INVLPG(addr) __asm__ volatile ("invlpg (%0)" : : "b"(addr) : "memory")

struct allocator default_frame_allocator;

__init void initialize_frame_allocation()
{
    uint64_t end = mem_map.area[mem_map.count - 1].end;
    alloc_init(&default_frame_allocator, KERNEL_END, end,
               (void*) 0xFFFFFFFF80000000, (void*) 0xffffff7fffc00000);
    print_frame_allocation_info();
}

void print_frame_allocation_info()
{
    struct allocator *a = &default_frame_allocator;
    printk("Begin: %lx\tEnd: %lx\n", a->begin, a->end);
    printk("Size:  %ld KiB\nCapacity: %ld KiB\n", a->size / 1024, a->capacity / 1024);
    printk("Free:  %ld KiB\n", a->free / 1024);
}

static int64_t intersection(uint64_t beginA, uint64_t endA, uint64_t beginB, uint64_t endB)
{
    uint64_t maxBegin = beginA > beginB ? beginA : beginB;
    uint64_t minEnd = endA < endB ? endA : endB;

    int64_t diff = minEnd - maxBegin;
    return diff > 0 ? diff : 0;
}

static void zero_frame(uint64_t frame_addr)
{
    assert(frame_addr % PAGE_SIZE == 0);
    // TODO use mechanism like kmap
    uint64_t *frame = (uint64_t *)0xFFFFFFFF80001000;
    uint64_t *frame_pte = (uint64_t *)0xffffff7fffc00008;
    *frame_pte = frame_addr | 3;
    INVLPG(frame);
    memset(frame, 0, PAGE_SIZE);
}

void alloc_init(struct allocator *alloc, uint64_t begin, uint64_t end,
                void *current_address, uint64_t *current_pte)
{
    assert(begin % PAGE_SIZE == 0);
    assert(end % PAGE_SIZE == 0);
    assert(end > begin);

    alloc->free = 0;
    // set free to amount of usable memory according to map
    for (int i = 0; i < mem_map.count; ++i) {
        struct area *area = mem_map.area + i;
        alloc->free += intersection(begin, end, area->begin, area->end);
    }

    // set first relevant map entry index
    for (int i = 0; i < mem_map.count; ++i) {
        if (mem_map.area[i].end > begin) {
            alloc->current_map_position = i;
            break;
        }
    }

    // move begin if starts in unavailable memory
    if (begin < mem_map.area[alloc->current_map_position].begin) {
        begin = mem_map.area[alloc->current_map_position].begin;
    }

    alloc->begin = begin;
    alloc->end = end;
    alloc->size = end - begin;
    alloc->capacity = alloc->free;
    alloc->current = NULL;
    alloc->count = 0;
    alloc->current_address = current_address;
    alloc->current_pte = current_pte;
    *alloc->current_pte = 3; // present, rw
}

static uint64_t extend(struct allocator *alloc)
{
    // if begin reached end of current memory area, jump to next area
    if (alloc->begin == mem_map.area[alloc->current_map_position].end) {
        alloc->current_map_position += 1;
        alloc->begin = mem_map.area[alloc->current_map_position].begin;
    }
    uint64_t frame = alloc->begin;
    alloc->begin += 4096;
    return frame;
}

// gets physical address of currently mapped node
static uint64_t current(struct allocator *alloc)
{
    return *alloc->current_pte & ~0xFFF;
}

static struct allocator_node *map(struct allocator *alloc, uint64_t addr)
{
    INVLPG(alloc->current_address);
    if (!addr) {
        return NULL;
    }
    *alloc->current_pte &= 0xFFF;
    *alloc->current_pte |= addr;
    return alloc->current_address;
}

uint64_t alloc_frame(struct allocator *alloc)
{
    if (alloc->free == 0) {
        return 0;
    }

    uint64_t frame = -1;

    if (alloc->count > 1) {
        frame = alloc->current->frames[alloc->count - 2];
        alloc->count -= 1;
    }

    else if (alloc->count == 1) {
        frame = current(alloc);
        alloc->current = map(alloc, alloc->current->next);
        if (alloc->current) {
            alloc->count = 512;
        }
        else {
            alloc->count = 0;
        }
    }

    else if (alloc->count == 0) {
        frame = extend(alloc);
    }

    assert(frame != (uint64_t)-1);

    alloc->free -= PAGE_SIZE;
    zero_frame(frame);
    return frame;
}

void dealloc_frame(struct allocator *alloc, uint64_t frame)
{
    assert(frame % 4096 == 0);
    if (alloc->count == 0 || alloc->count == 512) {
        uint64_t next = current(alloc);
        alloc->current = map(alloc, frame);
        alloc->current->next = next;
        alloc->count = 1;
    }
    else {
        alloc->current->frames[alloc->count - 1] = frame;
        alloc->count += 1;
    }
    alloc->free += PAGE_SIZE;
    assert(alloc->free <= alloc->size);
}

uint64_t get_frame()
{
    return alloc_frame(&default_frame_allocator);
}

void free_frame(uint64_t frame)
{
    dealloc_frame(&default_frame_allocator, frame);
}
