#!/bin/bash
set -e
rm -f *.o *.bin *.rom

as --32 reset.S   -o reset.o
#as --32 start16.S -o start16.o
gcc -m32 -c start16.S -o start16.o
as --32 start32.S -o start32.o
#as --32 boot.S -o boot.o
gcc -m32 -c kmain.c -o kmain.o
gcc -m32 -c kernel.c -o kernel.o

ld -m elf_i386 \
   -T linker.ld \
   -nostdlib \
   reset.o start16.o start32.o kmain.o kernel.o \
   -o bios.elf

objcopy -O binary bios.elf bios.rom

# ROM must be 64KB
truncate -s 1048576 bios.rom
#objcopy -O binary --gap-fill=0xFF bios.elf bios.bin
#objcopy -O binary --gap-fill=0xFF --pad-to=0x100000 bios.elf bios.bin

