NM=nasm
AR=i386-elf-ar
CC=i386-elf-gcc
LD=i386-elf-ld
LIB=-I ../lib/ -I ../userprog/ -I ../fs/ -I ../thread/ -I ../kernel/
CFLAGS = -Wall $(LIB) -c -fno-builtin -w -Wstrict-prototypes \
		 -Wmissing-prototypes -Werror -Wimplicit-function-declaration -fno-stack-protector \
		 -m32 -g -DUSER
external=../build/
EX_OBJ=$(external)/string.o \
	   $(external)/syscall.o \
	   $(external)/stdio.o \
	   $(external)/assert.o

start.o: start.S
	$(NM) -f elf $^ -o $@

simple_crt.a: $(EX_OBJ) start.o
	$(AR) rcs $@ $^

_%: %.o simple_crt.a
	$(LD)  -o $@ $^

.PRECIOUS: %.o


clean:
	rm $(BIN)


.PHONY: clean
