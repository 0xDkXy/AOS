CC=i386-elf-gcc
LD=i386-elf-ld
LIB=-I ../lib/
CFLAGS = -Wall $(LIB) -c -fno-builtin -w -Wstrict-prototypes \
		 -Wmissing-prototypes -Werror -Wimplicit-function-declaration -fno-stack-protector \
		 -m32 -g -DUSER
external=../build/
EX_OBJ=$(external)/string.o \
	   $(external)/syscall.o \
	   $(external)/stdio.o \
	   $(external)/assert.o

BIN=_prog_no_arg


_%: %.o $(EX_OBJ)
	$(LD) -e main -o $@ $^

.PRECIOUS: %.o

.DEFAULT_GOAL := $(BIN)

clean:
	rm $(BIN)


.PHONY: clean
