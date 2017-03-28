export CC        = x86_64-elf-gcc
export CFLAGS    = -ffreestanding -mcmodel=kernel -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -Wall -Wextra -Werror=return-type -Og -std=c11 -I${PWD}/include -I${PWD}/libc/include
export AFLAGS    = -felf64
export LDFLAGS   = -ffreestanding -mcmodel=kernel -z max-page-size=0x1000 -T arch/x64/linker.ld -nostdlib -lgcc
OBJS             =              \
	arch/x64/boot.o             \
	arch/x64/init.o             \
	arch/x64/irq.o              \
	arch/x64/irq_asm.o          \
	arch/x64/multiboot.o        \
	arch/x64/io/tty.o           \
	arch/x64/io/ports.o         \
	core/printk.o               \
	core/task.o                 \
	core/task_asm.o             \
	libc/string.o               \
	mm/alloc.o                  \
	mm/buddy.o                  \
	mm/frame.o                  \
	mm/memory_map.o             \
	mm/pages.o                  \



KERNEL    = kernel.a
IMAGE     = kernel.elf
ISO       = os.iso

$(ISO): $(IMAGE)
	./buildiso.sh $(IMAGE) $(ISO)

$(IMAGE): $(KERNEL)
	$(CC) $(LDFLAGS) -o $@ $^

$(KERNEL): $(OBJS)
	ar rvs $@ $^

%.o: %.asm
	nasm $(AFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

image: $(IMAGE)

iso: $(ISO)

emulate: $(ISO)
	bochs -q

clean:
	#@make -C stdc clean
	rm -rvf $(KERNEL) $(IMAGE) $(ISO) $(OBJS) iso-build/ bochsout.txt

.PHONY: clean emulate image iso
