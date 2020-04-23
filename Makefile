CFLAGS = -Wall -Wextra -DTRACE -std=c89 -pedantic

ifdef RELEASE
OPTFLAGS = -fuse-linker-plugin -flto -O2 -fdata-sections -ffunction-sections -Wl,--gc-sections
endif

all: nanac.exe test

.PHONY: test
test: nanac.exe $(patsubst %.asm,%.test,$(wildcard test/*.asm))

libnanac.a: nanac_vm.o nanac_builtins.o
	$(AR) r $@ $+

%.o: %.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -o $@ -c $<

nanac.exe: main.o libnanac.a
	$(CC) $(CFLAGS) $(OPTFLAGS) -o $@ $< -L. -lnanac

clean:
	rm -f *.o *.bin *.exe *.pyc *.a test/*.bin test/*.c test/*.out test/*.exe test/*.exe.tst

%.bin: %.asm
	./assemble.py $<
	@echo ""

test/%.out: test/%.bin
	./nanac.exe $< > $@

test/%.c: test/%.bin tocfile.py
	./tocfile.py $< > $@

test/%.exe.out: test/%.exe
	./$< | cut -f 1 -d ' ' > test/$*.exe.out

test/%.exe.tst: test/%.tst
	cat test/$*.tst | cut -f 1 -d ' ' > test/$*.exe.tst

test/%.exe: test/%.c test/%.out
	$(CC) $(CFLAGS) $(OPTFLAGS) -I. -o $@ $< nanac_vm.c test_main.c

test/%.exe.diff: test/%.exe.tst test/%.exe.out
	diff test/$*.exe.tst test/$*.exe.out

test/%.test: test/%.diff test/%.exe.diff
	@echo "Done"

test/%.diff: test/%.out
	diff test/$*.tst $<
