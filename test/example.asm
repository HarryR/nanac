# Example

$ZERO 0   # Friendly names for registers
$ONE 1

:start	jmp to :main
:exit	jmp die
:main
	reg mov $ZERO $ONE
	reg swp $ONE $ZERO
	jmp set :exit
	jmp neq $ZERO $ONE

:end jmp eq $ZERO $ONE
