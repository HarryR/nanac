CFLAGS = -Wall --std=c99 -Os

all: nanac test.bin

nanac: core.o main.o
	$(CC) $(CFLAGS) -o $@ $+

clean:
	rm -f *.o *.bin nanac

%.bin: %.asm assemble.py
	./assemble.py $<