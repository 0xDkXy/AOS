.PHONY: clean

run: mbr 
	qemu-system-x86_64 hd60M.img 

debug: mbr 
	qemu-system-x86_64 -S -s hd60M.img

mbr: mbr.bin
	dd if=build/mbr.bin of=hd60M.img bs=512 count=1 conv=notrunc

mbr.bin: boot/mbr.S
	nasm -I ./boot/include/ -o build/mbr.bin boot/mbr.S

init:
	qemu-img create -f raw hd60M.img 60M ;\
		mkdir build

clean: 
	rm -r build *.img
