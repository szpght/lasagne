#include <mm/buddy.h>
#include <stdbool.h>
#include <printk.h>

static int log2(long number);
static void flip_bit(uint8_t *buffer, int bit);
static uint8_t get_bit(uint8_t *buffer, int bit);
static void set_bit(uint8_t *buffer, int bit);
static void reset_bit(uint8_t *buffer, int bit);
static bool is_power_of_2(size_t x);
static int level_from_size(struct allocator *alloc, size_t size);
static size_t size_from_level(struct allocator *alloc, int level);
static struct allocator_node *create_block(struct allocator *alloc, int level);
static bool not_empty(struct allocator_node *block);
static bool empty(struct allocator_node *block);
static void split(struct allocator *alloc, struct allocator_node *block, int level);
static int unique_index(int level, int index_in_level);
static int index_in_level(struct allocator *alloc, void *block, int level);
static void flip_allocation_bit(struct allocator *alloc, void* block, int level);
static int allocation_bit_index(struct allocator *alloc, void* block, int level);
static uint8_t get_allocation_bit(struct allocator *alloc, void* block, int level);
static void *calculate_buddy_address(struct allocator *alloc, void *block, int level);
static void remove_from_list(struct allocator *alloc, struct allocator_node *block, int level);
static int allocation_level(struct allocator *alloc, void *block);


void allocator_print_status(struct allocator *alloc)
{
    printk("STATUS OF ALLOCATOR AT %lx\n", alloc);
    printk("Memory:     %lx-%lx\n", alloc->memory, alloc->memory + alloc->size - 1);
    printk("Free space: %ld b\n", alloc->free_size);
    printk("Leaf size:  %ld b\n", alloc->leaf_size);
    printk("Free blocks:\n");
    for (int i = 0; i <= alloc->max_level; ++i) {
        printk("%ld: %ld\n", size_from_level(alloc, i), alloc->free_count[i]);
    }
}

void allocator_init(struct allocator *alloc, void *memory, size_t size,
                    size_t leaf_size, void *bitmaps)
{
    alloc->memory = memory;
    alloc->size = size;
    alloc->free_size = 0;
    alloc->leaf_size = leaf_size;
    alloc->max_level = log2(size / leaf_size);
    alloc->bitmap_entries = (1 << alloc->max_level) - 1;
    int bitmap_size = alloc->bitmap_entries / 8 + 1;
    if (bitmaps == ALLOCATOR_EMPTY) {
        bitmaps = memory;
    }
    alloc->allocation_bitmap = bitmaps;
    alloc->split_bitmap = bitmaps + bitmap_size;

    for (int i = 0; i < ALLOCATOR_MAX_LEVELS; ++i) {
        alloc->free_blocks[i] = ALLOCATOR_EMPTY;
        alloc->free_count[i] = 0;
    }
    for (int i = 0; i < bitmap_size; ++i) {
        alloc->allocation_bitmap[i] = 0;
        alloc->split_bitmap[i] = 0xFF;
    }
}

size_t allocator_bitmaps_size(size_t memory_size, size_t leaf_size)
{
    return memory_size / leaf_size * 2;
}

int allocator_init_free(struct allocator *alloc, void *begin, void *end)
{
    // sanity checks
    size_t offset = begin - alloc->memory;
    if (begin < alloc->memory
        || offset % alloc->leaf_size) {
            return 1;
    }
    if (end > alloc->memory + alloc->size) {
        end = alloc->memory + alloc->size;
    }

    void *bitmap_begin = alloc->allocation_bitmap;
    void *bitmap_end = alloc->split_bitmap + (alloc->split_bitmap - alloc->allocation_bitmap);
    while (begin < end) {
        if (begin >= bitmap_begin && begin < bitmap_end) {
            begin = bitmap_end;
            continue;
        }
        allocator_deallocate_fast(alloc, begin, alloc->leaf_size);
        begin += alloc->leaf_size;
    }
    return 0;
}

void allocator_init_free_auto(struct allocator *alloc)
{
    allocator_init_free(alloc, alloc->memory, alloc->memory + alloc->size);
}

void *allocator_allocate(struct allocator *alloc, size_t size)
{
    // sanity checks
    if (!is_power_of_2(size) || size > alloc->free_size) {
        return ALLOCATOR_EMPTY;
    }
    int level = level_from_size(alloc, size);

    struct allocator_node *block = alloc->free_blocks[level];
    if (not_empty(block)) {
        if (not_empty(block->next)) {
            block->next->prev = ALLOCATOR_EMPTY;
        }
        alloc->free_blocks[level] = block->next;
        alloc->free_count[level] -= 1;
    }
    else {
        block = create_block(alloc, level);
    }

    // toggle allocation bit for parent block
    flip_allocation_bit(alloc, block, level);
    if (not_empty(block)) {
        alloc->free_size -= size;
    }
    return block;
}

static void flip_allocation_bit(struct allocator *alloc, void* block, int level)
{
    // TODO let's see what happens if we ignore 0 level bit
    if (level == 0) {
        return;
    }

    int index = allocation_bit_index(alloc, block, level);
    flip_bit(alloc->allocation_bitmap, index);
}

static uint8_t get_allocation_bit(struct allocator *alloc, void* block, int level)
{
    int index = allocation_bit_index(alloc, block, level);
    return get_bit(alloc->allocation_bitmap, index);
}

static int allocation_bit_index(struct allocator *alloc, void* block, int level)
{
    int level_index = index_in_level(alloc, block, level);
    return unique_index(level - 1, level_index / 2);
}

static int split_bit_index(struct allocator *allocator, void *block, int level)
{
    int level_index = index_in_level(allocator, block, level);
    return unique_index(level, level_index);
}

static struct allocator_node *create_block(struct allocator *alloc, int level)
{
    // get highest level free block
    struct allocator_node *block;
    int source_level = level;
    do {
        source_level -= 1;
        if (source_level < 0) {
            return ALLOCATOR_EMPTY;
        }
        block = alloc->free_blocks[source_level];
    } while (empty(block));

    // remove block from list
    alloc->free_blocks[source_level] = block->next;
    alloc->free_count[source_level] -= 1;

    // continue splitting block until you get deserved size
    while (source_level < level) {
        split(alloc, block, source_level);
        source_level += 1;
    }
    return block;
}

/* *
 * Splits block into two and adds second one to list
 */
static void split(struct allocator *alloc, struct allocator_node *block, int level)
{
    // get new block pointer
    size_t size = size_from_level(alloc, level);
    size_t new_size = size / 2;
    int new_level = level + 1;
    struct allocator_node *new_block = (void*)block + new_size;

    // mark parent block as split
    int index = split_bit_index(alloc, block, level);
    set_bit(alloc->split_bitmap, index);
    // mark block as allocated
    flip_allocation_bit(alloc, block, level);

    // set new block neighbours
    new_block->prev = ALLOCATOR_EMPTY;
    new_block->next = alloc->free_blocks[new_level];

    // add block to list
    alloc->free_blocks[new_level] = new_block;
    if (not_empty(new_block->next)) {
        new_block->next->prev = new_block;
    }
    alloc->free_count[new_level] += 1;
}

static void remove_from_list(struct allocator *alloc, struct allocator_node *block, int level)
{
    if (not_empty(block->next)) {
        block->next->prev = block->prev;
    }
    if (not_empty(block->prev)) {
        block->prev->next = block->next;
    }
    else {
        alloc->free_blocks[level] = block->next;
    }
    alloc->free_count[level] -= 1;
}

void allocator_deallocate_fast(struct allocator *alloc, void *block, size_t size)
{
    int level = level_from_size(alloc, size);
    struct allocator_node *base_addr = block;
    do {
        flip_allocation_bit(alloc, base_addr, level);

        // return if buddy allocated
        if (get_allocation_bit(alloc, base_addr, level)) {
            goto end;
        }
        void *buddy = calculate_buddy_address(alloc, base_addr, level);
        remove_from_list(alloc, buddy, level);
        int parent_split_bit = split_bit_index(alloc, base_addr, level - 1);
        reset_bit(alloc->split_bitmap, parent_split_bit);
        base_addr = base_addr < buddy ? base_addr : buddy;
        level -= 1;
    } while (level > 0);

    end:
    alloc->free_size += size;
    // add new free block to list
    if (not_empty(alloc->free_blocks[level])) {
        alloc->free_blocks[level]->prev = base_addr;
    }
    base_addr->next = alloc->free_blocks[level];
    base_addr->prev = ALLOCATOR_EMPTY;
    alloc->free_blocks[level] = base_addr;
    alloc->free_count[level] += 1;
}

void allocator_deallocate(struct allocator *alloc, void *block)
{
    // get size of allocation
    int level = allocation_level(alloc, block);
    size_t size = alloc->leaf_size;
    size *= 1 << (alloc->max_level - level);
    allocator_deallocate_fast(alloc, block, size);
}

static int allocation_level(struct allocator *alloc, void *block)
{
    int level = alloc->max_level;
    while (level > 0) {
        int bit = split_bit_index(alloc, block, level - 1);
        if (get_bit(alloc->split_bitmap, bit)) {
            return level;
        }
        level -= 1;
    }
    return 0;
}

static void *calculate_buddy_address(struct allocator *alloc, void *block, int level)
{
    int64_t relative_addr = block - alloc->memory;
    int bit = (alloc->max_level - level);
    relative_addr ^= (1 << bit) * alloc->leaf_size;
    return relative_addr + alloc->memory;
}

static int unique_index(int level, int level_index)
{
    return (1 << level) + level_index - 1;
}

static int index_in_level(struct allocator *alloc, void *block, int level)
{
    return (block - alloc->memory) / size_from_level(alloc, level);
}

static bool not_empty(struct allocator_node *block)
{
    return block != ALLOCATOR_EMPTY;
}

static bool empty(struct allocator_node *block)
{
    return block == ALLOCATOR_EMPTY;
}

static int level_from_size(struct allocator *alloc, size_t size)
{
    int level = 0;
    while (size < alloc->size)
    {
        level += 1;
        size *= 2;
    }
    return level;
}

static size_t size_from_level(struct allocator *alloc, int level)
{
    return alloc->size / (1 << level);
}

///////// utils

static void flip_bit(uint8_t *buffer, int bit)
{
    int index = bit / 8;
    int offset = bit % 8;
    buffer[index] ^= (1 << offset);
}

static uint8_t get_bit(uint8_t *buffer, int bit)
{
    int index = bit / 8;
    int offset = bit % 8;
    return (buffer[index] >> (offset)) & 1;
}

static void set_bit(uint8_t *buffer, int bit)
{
    int index = bit / 8;
    int offset = bit % 8;
    buffer[index] |= (1 << offset);
}

static void reset_bit(uint8_t *buffer, int bit)
{
    int index = bit / 8;
    int offset = bit % 8;
    buffer[index] &= ~(1 << offset);
}

static int log2(long number)
{
    int result = -1;
    while (number) {
        result += 1;
        number /= 2;
    }
    return result;
}

static bool is_power_of_2(size_t x)
{
    return (x != 0) && !(x & (x - 1));
}