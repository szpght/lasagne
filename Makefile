include config.mk

IMGSUFFIX = .bin
IMG = $(IMGBASE)$(IMGSUFFIX)
ISO = $(IMGBASE).iso

$(ISO): lasagne/arch/$(ARCH)
	$(MAKE) IMG=$(IMG) ISO=$(ISO) -C $< $@
	mkdir -p $(@D)
	cp -u $</$@ $@

$(IMG): lasagne/arch/$(ARCH)
	$(MAKE) IMG=$(IMG) -C $< $@
	mkdir -p $(@D)
	cp -u $</$@ $@

clean:
	find \( -name '*.o' \
		  -or -name '*.bin' \
			-or -name '*.iso' \
			-or -name '*.tmp' \) -exec rm -v {} \;

kernel: $(IMG)

iso: $(ISO)

emulate:
	$(MAKE) -C lasagne/arch/$(ARCH) emulate

.PHONY: kernel iso emulate clean
