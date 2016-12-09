include $(ROOTDIR)/config.mk

CC        = i686-elf-gcc
CFLAGS    = -ffreestanding -Wall -Wextra -O0 -std=c11 -pedantic -I$(ROOTDIR)/include
LDFLAGS   = -T linker.ld -ffreestanding -Os -nostdlib -lgcc -fno-use-linker-plugin
NASM      = nasm

OBJS      = ../../kernel_main.o init.o io/tty.o frames.o
BOOTSTRAP = bootstrap.o

# Kernel
$(KERNEL): $(BOOTSTRAP) $(OBJS) linker.ld
	mkdir -p $(@D)
	$(CC) $(LDFLAGS) -o $@ $(BOOTSTRAP) $(OBJS)

$(ISO): $(KERNEL)
	mkdir -p $(@D)
	mkdir -p iso-build/boot/grub/
	cp -a $(KERNEL) iso-build/boot/
	(cd iso-build && sed 's#@KERNEL@#$(KERNEL)#g' ../grub.cfg.in > ../iso-build/boot/grub/grub.cfg)
	grub-mkrescue -o $(ISO) iso-build

# Object files
%.o: %.asm
	$(NASM) -felf32 $< -o $@

%.o: %c
	$(CC) -c $< -o $@ $(CFLAGS)

# Shorthands
kernel: $(KERNEL)

iso: $(ISO)

emulate: $(ISO)
	bochs $(BOCHSFLAGS) -f lasagne.bochsrc

clean:
	rm -rvf iso-build/ \
	        $(ISO) \
	        bochsout.txt \
	        parport.out \
	        *.o \
	        *.bin

.PHONY: kernel iso emulate clean
