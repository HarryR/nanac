CFLAGS = -Wall --std=c99 -Os -DTRACE

all: nanac.exe test.bin

nanac.exe: core.o main.o builtins.o
	$(CC) $(CFLAGS) -o $@ $+

clean:
	rm -f *.o *.bin *.exe *.pyc

%.bin: %.asm assemble.py
	./assemble.py $<