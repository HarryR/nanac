# Nanac

Nanac is a tiny Python two-pass assembler and a C virtual machine. The microcode VM is under 150 lines of C and is fully modular allowing modules to be added at runtime without changing the assembler or core VM code.

### Features

 * 256 registers, no stack, 16bit address space
 * Register windows for subroutines
 * Easy and quick to modify and experiment
 * No dynamic heap or stack allocation
 * Typeless registers, `void*`
 * Extremely portable C99 code
 * Auto-updating (dis)assembler
 * Highly modular, add opcodes at runtime
 * 16-bit microcontroller friendly


## Example.asm

```asm
$ZERO 0   # Friendly names for registers
$ONE 1

:start  jmp to :main
:exit   jmp die
:main
    reg mov $ZERO $ONE
    reg swp $ONE $ZERO
    jmp set :exit
    jmp neq $ZERO $ONE

:end jmp eq $ZERO $ONE

```

When executing it will generate a trace:

```asm
@0    jmp to 2 0
@2    reg mov 0 1
@3    reg swp 1 0
@4    jmp set 1 0
@5    jmp neq 0 1
@6    jmp eq 0 1
@1    jmp die 0 0
```

The assembler will generate a listing:

```asm
01010200 @0    jmp to :main   # test.asm:5
01000000 @1    jmp die    # test.asm:6
00000001 @2    reg mov $ZERO $ONE  # test.asm:8
00020100 @3    reg swp $ONE $ZERO  # test.asm:9
01020100 @4    jmp set :exit   # test.asm:10
01040001 @5    jmp neq $ZERO $ONE  # test.asm:11
01030001 @6    jmp eq $ZERO $ONE  # test.asm:13
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
static int jmp_eq( nanac_t *cpu, uint8_t arga, uint8_t argb ) {
    cpu->do_jump = nanac_reg_get(cpu, arga).ptr == nanac_reg_get(cpu, argb).ptr;
    return 0;
}
```

### Registering a native module

```c
    static const nanac_cmd_t _cmds_jmp[] = (nanac_cmd_t[]){
        {"eq", &jmp_eq},
    };
    nanac_mods_add(mods, "jmp", 1, _cmds_jmp);
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

There can be up to `2^16` operations (262140 bytes of code), the `eip` is 16bit.

Conditionals are handled by saving the 'jump address' into the `jip` special register, then calling jump instructions such as `jmp neq` and `jmp eq` which - if the condition matches - will set `eip` to `jip`, otherwise increment `eip` and continue.

Basic Modules:

 * `jmp` - Jump and control flow
 * `op` - Construct temporary opcodes
 * `reg` - Registers

Other modules must be implemented by the user.

### `jmp` module

 * `set :label` - Set `JIP` to an absolute address, offset in opcodes not bytes
 * `to :label` - Immediate jump to an absolute address
 * `eq $A $B` - If $A and $B are equal, jump to `JIP`
 * `neq $A $B` - If $A and $B aren't the same, jump to `JIP`
 * `or $A $B` - If either $A or $B aren't uninitialised, jump to `JIP`
 * `and $A $B` - If both $A and $B aren't uninitialised, jump to `JIP`
 * `die` - Terminate program
 * `ret $A $B` - Load opcode from register A, reduce register window by B, run temporary opcode

### `op` module

 * `jmp $A $B` - Create `jmp to $A $B` temporary instruction
 * `sav $A` - Save temporary instruction to register A

### `reg` module

 * `swp $A $B` - Swap register B with register A
 * `mov $A $B` - Copy register B to register A
 * `crl $A $B` - Reset registers A and B to an uninitialised state (e.g. `NULL`)
 * `win $A $B` - Increment register window by $A, decrement by $B