export CC        = x86_64-elf-gcc
export CFLAGS    = -ffreestanding -mcmodel=kernel -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -Wall -Wextra -O0 -std=c11 -I${PWD}/include
export AFLAGS    = -felf64
export LDFLAGS   = -ffreestanding -mcmodel=kernel -z max-page-size=0x1000 -T arch/x64/linker.ld -nostdlib -lgcc
OBJS      =                     \
	arch/x64/boot.o             \
	arch/x64/init.o             \
	arch/x64/io/tty.o           \
	arch/x64/io/ports.o         \
	base/printk.o               \



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

iso: $(ISO)

emulate: $(ISO)
	bochs -f os.bochsrc

clean:
	#@make -C stdc clean
	rm -rvf $(KERNEL) $(IMAGE) $(ISO) $(OBJS) iso-build/ bochsout.txt

.PHONY: clean emulate iso
