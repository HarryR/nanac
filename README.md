# Nanac

Nanac is a tiny single-pass assembler and virtual machine. The VM is under 150 lines of C and is fully modular, the machine uses on 32bit opcodes:

 * 8bit module ID
 * 8bit cmd ID
 * 8bit arg A
 * 8bit arg B

New operations are registered as modules containing commands in your programs source code,
the module and cmd IDs are printed to stdout for use by the assembler.

## Example Usage

```c
int main( int argc, char **argv ) {
	nanac_cpu_t cpu;
	nanac_init(&cpu);
	nanac_builtins(&cpu);

	print_mods(&cpu);

	if( argc > 1 ) {
		load_file(&cpu, argv[1]);
	}

	nanac_run(&cpu);

	if( cpu.ops ) {
		free(cpu.ops);
	}

	return 0;
}
```

## Example Program

```asm
# Example

$ZERO 0
$ONE 1

:start	jmp to 2
:exit	jmp die
:derp
	reg mov $ZERO $ONE
	reg swp $ONE $ZERO
	jmp set :exit
	jmp neq $ZERO $ONE

:end jmp eq $ZERO $ONE
```