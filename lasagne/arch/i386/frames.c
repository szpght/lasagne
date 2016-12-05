#include "memory_sections.h"
#include "frames.h"
#include <stdint.h>


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
		if (mmap_entry->type != 1) {
			goto next;
		}
		int start_bit = mmap_entry->base_addr / 4096;
		int bits = mmap_entry->length / 4096;

		for (int i = start_bit; i < start_bit + bits; i++) {
			set_bit(frame_bitmap, i, 1);
		}

		next:
		mmap_entry = (uint32_t)mmap_entry + mmap_entry->size + 4;
	}
}
