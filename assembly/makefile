all: hello64 hello32

hello64: hello64.s
	as $^ -o $@.o --64
	ld $@.o -o $@ -m elf_x86_64

hello32: hello32.s
	as $^ -o $@.o --32
	ld $@.o -o $@ -m elf_i386

test: hello64 hello32
	./hello64
	./hello32

clean:
	rm -f hello64 hello64.o
	rm -f hello32 hello32.o

.PHONY: all test clean
