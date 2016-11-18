include config.mk

IMGSUFFIX = .bin
IMG = $(IMGBASE)$(IMGSUFFIX)

$(IMG): lasagne/arch/$(ARCH)
	$(MAKE) IMG=$(IMG) -C $< $@
	mkdir -p $(@D)
	mv $</$@ $@

clean:
	find \( -name '*.o' -or -name '*.bin' \) -exec rm -v {} \;
