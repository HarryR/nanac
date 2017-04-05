# Control flow example
#
# Demonstrates how register windows and dynamic opcodes can implement control
# flow patterns typically found in procedural programs.
#

	op jmp :exit        # Create `jmp to :exit` opcode
	op sav 1            # Save into register 1
	jmp to :main

:exit
	jmp die				# Halt program

:main
	reg win 1           # Register 0 now maps to absolute register 1
	jmp ret 0 1         # will jmp to :exit and decrement register window by 1	
