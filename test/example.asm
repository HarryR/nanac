# Example

$ZERO 0   # Friendly names for registers
$ONE 1

:start	jmp to :main
:exit	jmp die
:main
	reg mov $ZERO $ONE
	reg swp $ONE $ZERO
	cnd neq $ZERO $ONE
	jmp to :exit

:end
	cnd eq $ZERO $ONE
	jmp to :exit
