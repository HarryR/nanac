CFLAGS = -Wall --std=c99 -Os -DTRACE -s -fomit-frame-pointer

all: nanac.exe test

.PHONY: test
test: nanac.exe $(patsubst %.asm,%.test,$(wildcard test/*.asm))

libnanac.a: core.o builtins.o
	$(AR) r $@ $+

nanac.exe: main.o libnanac.a
	$(CC) $(CFLAGS) -o $@ $< -L. -lnanac
	strip -R .note -R .comment $@

clean:
	rm -f *.o *.bin *.exe *.pyc *.a

%.bin: %.asm
	./assemble.py $<
	@echo ""

test/%.out: test/%.bin
	./nanac.exe $< > test/$*.out

test/%.test: test/%.out
	diff test/$*.tst test/$*.out
