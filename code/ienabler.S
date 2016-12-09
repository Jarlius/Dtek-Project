	# Enable interrupts

	.global enable_interrupt
enable_interrupt:
	ei
	jr	$ra
	nop

