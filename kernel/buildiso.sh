#!/bin/bash -e

if [ $# -ne 2 ]
then
    echo "Usage: buildiso.sh [IMAGE NAME] [ISO NAME]"
    exit 1
fi

IMAGE=$1
ISO=$2
mkdir iso-build/boot/grub -p
cat << EOF > iso-build/boot/grub/grub.cfg
set timeout=0

menuentry "Kernel" {
  multiboot2 /boot/$IMAGE
}
EOF
cp $IMAGE iso-build/boot
grub-mkrescue -o $ISO iso-build
