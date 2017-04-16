.begin one $A $B
# Merp
.var $A = (derp $B)
:derp
	.if $A == $B
	:merp
		$A = $B
	.elif $C == $D
		.var $DERP
		$B = $C
		.if $C == (derp $D)
			$C = $B
		.end
	.else
		$C = $D
	.end
	.while $B == $C
		$C = $B
	.end
	return $C # derp
.end
