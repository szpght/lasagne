#include "memory_sections.h"
#include "frames.h"
#include <stdint.h>
#include <kernel/tty.h>


struct frame_management_data mem_frame_data;


static char get_bit(void *address, uint64_t bit) {
	char *mmap = address;
	int byte = bit / 8;
	bit = bit % 8;
	return (*(mmap + byte) >> bit) & 1;
}


static char set_bit(void *address, uint64_t bit, char value) {
	char *mmap = address;
	int byte = bit / 8;
	bit = bit % 8;
	char old = *(mmap + byte);
	char new;
	if (value) {
		new = old | (1 << bit);
	}
	else {
		new = old & ~((1 << bit));
	}
	*(mmap + byte) = new;
}


void fill_frame_bitmap_from_memmap(void *boot_information) {
	uint32_t *mmap_length = boot_information + 44;
	struct mmap_entry **mmap_entry_ptr = boot_information + 48;
	struct mmap_entry *mmap_entry = *mmap_entry_ptr;
	struct mmap_entry *mmap_end = (uint32_t)mmap_entry + *mmap_length;

	while (mmap_entry < mmap_end) {
		char available = mmap_entry->type == 1 ? FRAME_FREE : FRAME_USED;
		int start_bit = mmap_entry->base_addr / 4096;
		int bits = mmap_entry->length / 4096;

		// TODO don't use dirty hacks
		if (available) {
			struct frame_management_data *frames_data = (uint32_t)&mem_frame_data - (uint32_t)vmem_base;
			frames_data->frames = start_bit + bits;
		}

		for (int i = start_bit; i < start_bit + bits; i++) {
			set_bit(frame_bitmap, i, available);
		}

		mmap_entry = (uint32_t)mmap_entry + mmap_entry->size + 4;
	}
}


static void set_high_mem_first() {
	mem_frame_data.high_mem_first = LOWER_MEMORY_SIZE / FRAME_SIZE;
}


static void set_used_frames() {
	void *bitmap = frame_bitmap + (uint32_t)vmem_base;
	int start = (int)multiboot_header / FRAME_SIZE;
	int end = (int)free_memory_start / FRAME_SIZE;

	for (int i = start; i < end; i++) {
		set_bit(bitmap, i, FRAME_USED);
	}
}


static void set_next_free_frame() {
	mem_frame_data.next_free_frame = (int)free_memory_start / FRAME_SIZE;
}


void mem_initialize_frame_management() {
	set_high_mem_first();
	set_used_frames();
	set_next_free_frame();
}
