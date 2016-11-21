include config.mk

$(ISO): $(ARCHDIR)
	$(MAKE) -C $< $@
	mkdir -p $(@D)
	cp -u $</$@ $@

$(KERNEL): $(ARCHDIR)
	$(MAKE) -C $< $@
	mkdir -p $(@D)
	cp -u $</$@ $@

clean:
	find \( -name '*.o' \
	    -or -name '*.bin' \
	    -or -name '*.iso' \
	    -or -name '*.tmp' \) -exec rm -v {} \;
	$(MAKE) -C $(ARCHDIR) clean

kernel: $(KERNEL)

iso: $(ISO)

emulate:
	$(MAKE) -C $(ARCHDIR) emulate

.PHONY: kernel iso emulate clean
