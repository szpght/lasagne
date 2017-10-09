#!/bin/sh
ELF_FILE=$1
ISO_FILE=$2
qemu-system-x86_64 -s -S -cdrom $ISO_FILE &
QEMU_PID=$!
gdb $ELF_FILE -batch -ex 'target remote localhost:1234' -ex 'b initialize' -ex 'c' -ex 'disconnect'
#gdb `find . -type d -printf '-d %p '` $ELF_FILE -ex 'set arch i386:x86-64' -ex 'target remote localhost:1234'
gdb $ELF_FILE -ex 'set arch i386:x86-64' -ex 'target remote localhost:1234' -ex 'tui enable' -ex 'layout src'
kill $QEMU_PID
