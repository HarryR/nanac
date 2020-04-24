# Nanac

Nanac is a tiny C virtual machine, with a small Python two-pass assembler. The portable microcode VM is under 200 lines of C and is fully modular, which allows modules to be added at fruntime or compile-time without changing the assembler or core VM code.

### Features

 * 256 registers, no stack, 16bit address space
 * Register windows for subroutines
 * Easy and quick to modify and experiment
 * Typeless registers, `void*`, for extensibility
 * No heap allocation in VM core
 * Microcontroller friendly
 * Clean portable C89 / ANSI-C code
 * Auto-updating (dis)assembler
 * Bytecode to C translator
 * Highly modular (can even add opcodes at runtime)


## Example.asm

```asm
$ZERO 0   # Friendly names for registers
$ONE 1

:start  jmp to :main
:exit   jmp die
:main
    reg mov $ZERO $ONE
    reg swp $ONE $ZERO
    cnd neq $ZERO $ONE
    jmp to :exit

:end
    cnd eq $ZERO $ONE
    jmp to :exit
```

When executing it will generate a trace:

```asm
@0    jmp to 2 0
@2    reg mov 0 1
@3    reg swp 1 0
@4    cnd neq 0 1
@6    cnd eq 0 1
@7    jmp to 1 0
@1    jmp die 0 0
```

The assembler will generate a listing:

```asm
00000200 @0    jmp to :main   # test/example.asm:5
00010000 @1    jmp die    # test/example.asm:6
02000001 @2    reg mov $ZERO $ONE  # test/example.asm:8
02020100 @3    reg swp $ONE $ZERO  # test/example.asm:9
01010001 @4    cnd neq $ZERO $ONE  # test/example.asm:10
00000100 @5    jmp to :exit   # test/example.asm:11
01000001 @6    cnd eq $ZERO $ONE  # test/example.asm:14
00000100 @7    jmp to :exit   # test/example.asm:15
```


## Using and Extending

The builtins and vm are contained in `libnanac.a` and `nanac.h` which can be
included in projects as a git submodule. Build with:

```
git submodule add https://github.com/HarryR/nanac
cc -o myproject.exe -Inanac myproject.c -Lnanac -lnanac
```

### Implementing a native command

All commands have the same interface, they are passed the CPU context pointer and
two 8 bit values from the data half of the opcode. Each opcode is 4 bytes, two
indicating the module and command, then two arbitrary arguments.

Access registers through `nanac_reg_get` and `nanac_reg_set`, all registers are
a union type called `nanac_reg_t`.

```c
int reg_mov( struct nanac_s *cpu, unsigned char arga, unsigned char argb )
{
    nanac_reg_set(cpu, arga, nanac_reg_get(cpu, argb));
    return NANAC_OK;
} 
```

### Registering a native module

```c
int jmp_to( struct nanac_s *cpu, unsigned char arga, unsigned char argb )
{
    // ...
    return NANAC_OK
}

static const nanac_cmd_t _cmds_jmp[4] = {
    {"to", &jmp_to},
    {"die", &jmp_die},
    {"sub", &jmp_sub},
    {"ret", &jmp_ret},
};
nanac_mods_add(mods, "jmp", 4, _cmds_jmp);
```

### Example host program

```c
int main( int argc, char **argv )
{
    struct nanac_mods_s mods;
    struct nanac_s ctx;
    int ret;

    nanac_mods_init(&mods);         /* setup standard built-in modules + commands */
    nanac_mods_builtins(&mods);
    
    nanac_init(&ctx, &mods);        /* initialise context/CPU with the modules */

    if( load_file(&ctx, argv[1]) )  /* load bytecode file into ctx.ops */
    {
        ret = nanac_run(&ctx);
        if( ctx.ops )
            free(ctx.ops);  /* free loaded file */
    }

    return ret;
}
```


## Opcodes and Instructions

 The machine uses 32bit operations encoded as 4 independent bytes:

 * module ID `uint8`
 * cmd ID `uint8`
 * arg A `uint8`
 * arg B `uint`

New operations are registered as modules containing commands in your programs source code,
the module and cmd IDs, when new modules are added the assembler & disassembler automatically update their instructions.

There are `2^8` registers, each is a native sized `void*` pointer.

There can be up to `2^16` operations (256 kilobytes of code), the instruction pointer `eip` is 16bit.

Basic Modules:

 * `jmp` - Jumps and subroutines
 * `cnd` - Conditionally execute the next opcode
 * `reg` - Register manipulation

Other modules must be implemented by the user.

### `jmp` module

 * `to :label` - Immediate jump to an absolute address
 * `die` - Terminate program
 * `sub :label` - Enter sub-routine, allowing for `ret` to return
 * `ret $A $B` - Load opcode from register A, reduce register window by B, run temporary opcode


### `cnd` module

 * `eq $A $B` - If $A and $B are equal, execute next instruction
 * `neq $A $B` - If $A and $B aren't the same, execute next instruction
 * `nil $A $B` - f either $A or $B are uninitialised (`NULL`), execute next instruction
 * `nz $A $B` - If either $A or $B are *not* uninitialised (`NULL`), execute next instruction

### `reg` module

 * `swp $A $B` - Swap register B with register A
 * `mov $A $B` - Copy register B to register A
 * `crl $A $B` - Reset registers A and B to an uninitialised state (e.g. `NULL`)
 * `win $A $B` - Increment register window by $A, decrement by $B