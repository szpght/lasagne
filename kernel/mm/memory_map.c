#include <mm/memory_map.h>
#include <assert.h>

void *KERNEL_END = &_KERNEL_END;


struct memory_map mem_map;

void mem_map_add_area(uint64_t begin, uint64_t end)
{
    assert (begin < end);
    if (mem_map.count == MAP_MAX)
        return;

    evil_tail_call_optimization:
    // check if new interval doesn't overlap
    for (int i = 0; i < mem_map.count; ++i) {
        if (begin <= mem_map.area[i].end && mem_map.area[i].begin <= end) { // if intervals overlap (or border)
            // merge them mercilessly
            begin = begin < mem_map.area[i].begin ? begin : mem_map.area[i].begin;
            end = end > mem_map.area[i].end ? end : mem_map.area[i].end;

            // remove current interval by moving last (if any) into current position
            mem_map.count -= 1;
            if (mem_map.count) {
                mem_map.area[i] = mem_map.area[mem_map.count];
            }
            goto evil_tail_call_optimization;
        }
    }

    // if nothing overlaps, add interval to list
    mem_map.area[mem_map.count].begin = begin;
    mem_map.area[mem_map.count].end = end;
    mem_map.count += 1;
}

void mem_map_sanitize()
{
    // sort areas
    for (int i = 0; i < mem_map.count; ++i) {
        struct area *min = &mem_map.area[i];
        for (int j = i + 1; j < mem_map.count; ++j) {
            if (mem_map.area[j].begin < min->begin) {
                min = &mem_map.area[j];
            }
        }
        struct area tmp = *min;
        *min = mem_map.area[i];
        mem_map.area[i] = tmp;
    }

    // align areas to 4 KiB
    for (int i = 0; i < mem_map.count; ++i) {
        struct area *area = mem_map.area + i;
        if (area->begin % 4096) {
            area->begin += 4096 - area->begin % 4096;
        }
        if (area->end % 4096) {
            area->end -= area->end % 4096;
        }
        // TODO support areas smaller than 4K
        assert(area->begin < area->end);
    }

    // set physical end
    mem_map.physical_end = mem_map.area[mem_map.count - 1].end;
}
