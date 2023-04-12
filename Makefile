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

build/kernel.bin: build/main.o
	i386-elf-ld build/main.o -Ttext 0xc0001500 -e main -o build/kernel.bin

build/main.o: kernel/main.c
	i386-elf-gcc -c -o build/main.o kernel/main.c

init:
	qemu-img create -f raw hd60M.img 60M ;\
		mkdir build


clean: 
	rm build/*
