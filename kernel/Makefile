export CC        = x86_64-elf-gcc
export CFLAGS    = -ffreestanding -mcmodel=small -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -Wall -Wextra -O0 -std=c11 -I${PWD}/include
export AFLAGS    = -felf64
export LDFLAGS   = -ffreestanding -mcmodel=small -z max-page-size=0x1000 -T arch/x64/linker.ld -nostdlib -lgcc -fno-use-linker-plugin

OBJS      =                     \
	arch/x64/boot.o             \


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
	$(CC) -c $< -o $@ $(CFLAGS)

iso: $(ISO)

emulate: $(ISO)
	bochs -f os.bochsrc

clean:
	#@make -C stdc clean
	rm -rvf $(KERNEL) $(IMAGE) $(ISO) $(OBJS) iso-build/ bochsout.txt

.PHONY: clean emulate iso
