# Lasagne

Lasagne is an attempt to create amd64 operating system.
At this moment it has 
- simple memory manager that supports lazy memory allocation
  at an arbitrary virtual address and buddy allocator for kernel heap
- interrupt handling API that enables you to register any function written in C
  as interrupt handler
- context switching (two hardcoded threads at this moment)

## How to build

### Requirements

To build the kernel:
- gcc cross compiler built according to [this dockerfile](https://github.com/szpght/docker/tree/master/gcc-cross-x86_64-elf)
- NASM

Additionally:
- grub-mkrescue (to build bootable .iso image)
- bochs (to automatically start emulation)

### Build and emulation

```sh
cd kernel
make image
make iso
make emulate
```

## Screenshots
[You can see screenshots on a wiki page](https://github.com/szpght/lasagne/wiki/Screenshots)
