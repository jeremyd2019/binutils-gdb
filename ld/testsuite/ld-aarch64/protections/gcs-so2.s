	.global	foo3
	.type	foo3, %function
foo3:
	sub	sp, sp, #16
	mov	w0, 9
	str	w0, [sp, 12]
	ldr	w0, [sp, 12]
	add	w0, w0, 4
	str	w0, [sp, 12]
	nop
	add	sp, sp, 16
	ret
	.size	foo3, .-foo3
	.global	bar3
	.type	bar3, %function
bar3:
	sub	sp, sp, #16
	mov	w0, 9
	str	w0, [sp, 12]
	ldr	w0, [sp, 12]
	add	w0, w0, 4
	str	w0, [sp, 12]
	nop
	add	sp, sp, 16
	ret
	.size	bar3, .-bar3

.include "gnu-note-properties-selectable-merged.inc"
