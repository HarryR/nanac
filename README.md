# Nanac

Nanac is a tiny Python two-pass assembler and a C virtual machine. The microcode VM is under 150 lines of C and is fully modular allowing modules to be added at runtime without changing the assembler or core VM code.

### Features

 * 256 registers, no stack, 16bit address space
 * Register windows for subroutines
 * Easy and quick to modify and experiment
 * Typeless registers, `void*`
 * No heap allocation in VM core
 * Microcontroller friendly
 * Clean portable C99 code
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
cc -o myproject -Inanac myproject.c -Lnanac -lnanac
```

### Implementing a native command

All commands have the same interface, they are passed the context pointer and
two 8 bit values from the data half of the opcode.

Access registers through `nanac_reg_get` and `nanac_reg_set`, all registers are
a union type called `nanac_reg_t`.

```c
int reg_mov( nanac_t *cpu, uint8_t arga, uint8_t argb )
{
    nanac_reg_set(cpu, arga, nanac_reg_get(cpu, argb));
    return NANAC_OK;
} 
```

### Registering a native module

```c
int jmp_to( nanac_t *cpu, uint8_t arga, uint8_t argb )
{
    // ...
}

static const nanac_cmd_t _cmds_jmp[] = (const nanac_cmd_t[]){
    {"to", &jmp_to},
    {"die", &jmp_die},
    {"sub", &jmp_sub},
    {"ret", &jmp_ret},
};
nanac_mods_add(mods, "jmp", 4, _cmds_jmp);
```

### Example host program

```c
int main( int argc, char **argv ) {
    nanac_mods_t mods;
    nanac_mods_init(&mods);
    nanac_mods_builtins(&mods);
    print_mods(&mods);
    
    nanac_t ctx;
    nanac_init(&ctx, &mods);
    load_file(&ctx, argv[i]);
    int ret = nanac_run(&ctx);
    if( ctx.ops ) {
        free(ctx.ops);
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