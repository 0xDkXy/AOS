.PHONY: clean

done: mbr loader kernel
	echo 'wrote to disk'

bochs: done
	bochs -f ./bochsrc.disk

run: done
	qemu-system-x86_64 hd60M.img 

debug: done
	qemu-system-x86_64 -S -s hd60M.img

kernel: build/kernel.bin
	dd if=build/kernel.bin of=hd60M.img bs=512 count=200 seek=9 conv=notrunc

mbr: mbr.bin
	dd if=build/mbr.bin of=hd60M.img bs=512 count=1 conv=notrunc

loader: loader.bin
	dd if=build/loader.bin of=hd60M.img bs=512 count=4 seek=2 conv=notrunc

loader.bin: boot/loader.S
	nasm -I ./boot/include/ -o build/loader.bin boot/loader.S

mbr.bin: boot/mbr.S
	nasm -I ./boot/include/ -o build/mbr.bin boot/mbr.S

print: lib/kernel/print.S
	nasm -f elf -o build/print.o lib/kernel/print.S

build/kernel.o: kernel/kernel.S
	nasm -f elf -o build/kernel.o kernel/kernel.S

build/main.o: kernel/main.c
	i386-elf-gcc -I lib/ -I kernel/ -c -fno-builtin -o build/main.o \
		kernel/main.c

build/interrupt.o: kernel/interrupt.c
	i386-elf-gcc -I lib/ -I kernel/ -c -fno-builtin -o build/interrupt.o \
		kernel/interrupt.c

build/init.o: kernel/init.c kernel/interrupt.h
	i386-elf-gcc -I lib/ -I kernel/ -c -fno-builtin -o build/init.o \
		kernel/init.c

build/kernel.bin: build/main.o print build/kernel.o build/init.o build/interrupt.o
	i386-elf-ld  -Ttext 0xc0001500 -e main -o build/kernel.bin \
		build/init.o build/interrupt.o build/print.o build/kernel.o



init:
	qemu-img create -f raw hd60M.img 60M ;\
		mkdir build


clean: 
	rm build/*
