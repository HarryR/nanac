# Example

$ZERO 0
$ONE 1

:label
	reg mov 0 1
:derp
	reg mov $ZERO $ONE
	reg swp $ONE $ZERO
	jmp set :derp
	jmp neq $ZERO $ONE

:end jmp eq $ZERO $ONE
