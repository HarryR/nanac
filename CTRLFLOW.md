when calling a subroutine the register window is increased
parameters are passed as the first registers in the window
when returning the window is decreased
the return value is stored in register $0
the return address must be stored somewhere

  reg win +1
  reg win -1
  jmp ret 

currently the return address can't be stored in registers, there is a temporary
jump address `jip`, but that's overwritten in normal control flow.

new instructions could be used to construct an operation and save it to a register 
and also jump to a register, this would allow for the return address to be put into
the new window's register 0, e.g.:

  # Encode `jmp to :after_jump` instruction
  op jmp :after_jump
  op sav 1
  jmp to :mysub
:after_jump
  # mysub returns to here, which exits
  jmp die

:mysub
  reg win 1

  jmp ret 1 0

This allows you to modify the value of registers by composing them with the 'op' commands and use them to augment program control.

The disadvantage is that 2 additional instructions are needed to encode the call before a jump, and an additional one in the subroutine to adjust the register window for its self - in addition to the 'call' and 'return' opcodes

The register window can be used to define local variables.

The native pointer size must be at least 32bits to store an opcode in a register, this potentially makes the approach unfavourable for 8 and 16bit microprocessors, in that case a union between a `void*` and a `nanac_op_t` would fix that.

The absolutely required commands are:

  op jmp - encode an absolute jmp to :label into special opcode register
  op sav - save opcode into register $N
  reg win - add arg1 and subtract arg2 from register window
  jmp reg - execute opcode in register pointed by arg1, reduce window by arg2

Additional commands for modifying the special opcode register:

  op lod - Load opcode from register $N
  op cmd - set Mod and Cmd IDs
  op arg - set arg1 & arg2
  op lbl - set arg1 & arg2 to :label

The following 'op' commands are optional, as they would allow the programs own
code to be modified at run-time. If the opcodes have been compiled to native 
code it would be infeasible to allow this.

  lod - load opcode at :label
  sav - save opcode to :label

