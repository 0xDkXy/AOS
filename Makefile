.PHONY: clean

run: done
	qemu-system-i386 hd60M.img 

debug: done
	qemu-system-i386 -S -s hd60M.img

done: mbr loader
	echo 'wrote to disk'

mbr: mbr.bin
	dd if=build/mbr.bin of=hd60M.img bs=512 count=1 conv=notrunc

loader: loader.bin
	dd if=build/loader.bin of=hd60M.img bs=512 count=1 seek=2 conv=notrunc

loader.bin: boot/loader.S
	nasm -g  -I ./boot/include/ -o build/loader.bin boot/loader.S

mbr.bin: boot/mbr.S
	nasm -g  -I ./boot/include/ -o build/mbr.bin boot/mbr.S

init:
	qemu-img create -f raw hd60M.img 60M ;\
		mkdir build


clean: 
	rm build/*
