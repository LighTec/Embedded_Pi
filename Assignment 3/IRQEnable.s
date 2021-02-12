		.text
		.align 4
		.global enableInterrupts
enableInterrupts: msr	DAIFClr, 0b0010
		ret
