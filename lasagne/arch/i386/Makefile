CC 				= i686-elf-gcc
CFLAGS 		= -ffreestanding -Wall -Wextra -O0 -std=c11 -pedantic
LDFLAGS 	= -T linker.ld -ffreestanding -Os -nostdlib -lgcc -fno-use-linker-plugin
NASM    	= nasm

OBJS 			= main.o

BOOTSTRAP = bootstrap.o

# Kernel
$(IMG): $(BOOSTRAP) $(OBJS)
	mkdir -p $(@D)
	$(CC) $(LDFLAGS) -o $@ $^

$(ISO): $(IMG)
	mkdir -p $(@D)
	mkdir -p iso-build/boot/grub/
	cp -a $(IMG) iso-build/boot/
	(cd iso-build && sed 's#@IMG@#$(IMG)#g' ../grub.cfg.in > ../iso-build/boot/grub/grub.cfg)
	grub-mkrescue -o $(ISO) iso-build

# Object files
%.o: %.asm
	$(NASM) -felf32 $< -o $@

%.o: %c
	$(CC) -c $< -o $@ $(CFLAGS)

# Shorthands
kernel: $(IMG)

iso: $(ISO)

emulate: $(ISO)
	bochs -f lasagne.bochsrc

.PHONY: kernel iso emulate
