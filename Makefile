CFLAGS = -Wall --std=c99 -Os -DTRACE -s -fomit-frame-pointer

all: nanac.exe test

.PHONY: test
test: nanac.exe test.bin
	./nanac.exe test.bin

nanac.exe: core.o main.o builtins.o
	$(CC) $(CFLAGS) -o $@ $+
	strip -R .note -R .comment $@

clean:
	rm -f *.o *.bin *.exe *.pyc

%.bin: %.asm assemble.py
	./assemble.py $<
