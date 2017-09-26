export CC        = x86_64-elf-gcc
export CFLAGS_BASE = -ffreestanding -mno-mmx -mno-sse -mno-sse2 -fno-asynchronous-unwind-tables \
	-Wall -Wextra -Werror=return-type -O0 -std=c11 -I${PWD}/include -I${PWD}/libc/include
export CFLAGS    =  -mcmodel=kernel -mno-red-zone
export AFLAGS    = -felf64
export LDFLAGS   = -ffreestanding -mcmodel=kernel -z max-page-size=0x1000 -T arch/x64/linker.ld -nostdlib -lgcc
OBJS             =              \
	arch/x64/boot.o             \
	arch/x64/init.o             \
	arch/x64/irq.o              \
	arch/x64/irq_asm.o          \
	arch/x64/multiboot.o        \
	arch/x64/io/tty.o           \
	arch/x64/io/pit.o           \
	arch/x64/io/ports.o         \
	core/assert.o               \
	core/printk.o               \
	core/task_asm.o             \
	libc/string.o               \
	mm/frame.o                  \
	mm/memory_map.o             \
	mm/pages.o                  \
	mm/alloc.o                  \
	mm/buddy.o                  \
	core/task.o                 \

OBJS32           =              \
	arch/x64/paging_init32.o    \

KERNEL    = kernel.a
IMAGE     = kernel.elf
ISO       = os.iso
ELF32     = elf32

$(IMAGE): $(KERNEL)
	$(CC) $(LDFLAGS) -o $@ $^

$(ELF32): $(OBJS32)
	objcopy -O elf64-x86-64 $<

$(KERNEL): $(OBJS) $(ELF32) $(OBJS32)
	ar rvs $@ $(filter-out $(ELF32),$^)

$(ISO): $(IMAGE)
	./buildiso.sh $(IMAGE) $(ISO)

$(OBJS32): EXTRA_FLAGS := -m32

$(OBJS): EXTRA_FLAGS := $(CFLAGS)

%.o: %.asm
	nasm $(AFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS_BASE) $(EXTRA_FLAGS) -c $< -o $@

image: $(IMAGE)

iso: $(ISO)

emulate: $(ISO)
	bochs -q

clean:
	#@make -C stdc clean
	rm -rvf $(KERNEL) $(IMAGE) $(ISO) $(OBJS) $(OBJS32) iso-build/ bochsout.txt

.PHONY: clean emulate image iso elf32
