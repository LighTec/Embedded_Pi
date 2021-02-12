/*
*-----------------------------------------------*
*						*
*	   CPSC 355 Assignment 6		*
*		Kell Larson			*
*	         30056026			*
*						*
*TODO:						*
*[100% done] Input File Pointers		*
*[100% done] File Input loop			*
*[100% done] Seperate cube root subroutine	*
*[100% done] Double printf output		*
*-----------------------------------------------*
*/
define(fd_r, w19)
define(filepointer, x20)
define(i, x21)
define(nread_r, x22)
define(buf_base_r, x23)
define(x, d20)
define(y, d21)
define(dy, d23)
define(errmax, d24)

	buf_size = 8
	alloc = -(16 + buf_size) & -16
	dealloc = -alloc
	buf_s = 16

	.data
tolerance: .double 0r1.0e-10

	.text
header:	.string	"Input\t\tCube Root\n"
tbl:	.string "%f\t\t%f\n"
failError:	.string "\nFailed opening file. Exiting!\n"

	.balign 4
	.global main
main:	stp x29, x30, [sp, alloc]!
	mov x29, sp

	mov w19, 1		// the first arg is the program name.
				// we want the 2rd arg, the number entered

	ldr filepointer, [x1, w19, SXTW 3]	// x1 is the base addr to the external pointer array
				// containing pointers to all our args. w19 (1) is the index, SXTW 3
				// calculates the offset

	mov w0, -100		// set first argument as local folder
	mov x1, filepointer	// set second argument as file pointer
	mov w2, 0		// set third arg as 0
	mov w3, 0		// set fourth arg as 0
	mov x8, 56		// openat system request
	svc 0			// call system
	mov fd_r, w0
	cmp w0, 0		// check if good
	b.lt failOpen		// goto end & print error if failed

	adrp x0, header		// setting up table header string
	add x0, x0, :lo12:header
	bl printf		// print header string

	add buf_base_r, x29, buf_s	// compute buffer base address

	// read floats in a loop and prints them out
loop:
	mov w0, fd_r		// first arg
	mov x1, buf_base_r	// second arg (pointer to buffer + 16)
	mov w2, buf_size	// 3 arg (n)
	mov x8, 63		// read I/O system request
	svc 0			// call system function
	mov nread_r, x0		// save returned value to dedicated register
	
	cmp nread_r, buf_size	// check if bytes read = buffer (checks EOF)
	b.ne end

	ldr d0, [buf_base_r]	// get double
	fmov d5, d0		// save original
	bl getCube		// cube root is returned at d0
brk3:	
	fmov d1, d0		// move cube root to d1
	fmov d0, d5		// move original value back to d0

	// print out content read
	adrp x0, tbl		// get template string
	add x0, x0, :lo12:tbl
	bl printf		// print double
	
	b loop			// keep looping

failOpen:
	adrp x0, failError	// set up error message string
	add x0, x0, :lo12:failError
	bl printf		// print error message
end:
	ldp x29, x30, [sp], dealloc
	ret


getCube:	// given a variable at d0, returns its approximate cube root at d0 within 1e-10 error
		// uses Newton's method
	stp x29, x30, [sp, -16]!
	mov x29, sp
	
	adrp x28, tolerance
	add x28, x28, :lo12:tolerance
	ldr errmax, [x28]	// maximum error to keep looping for

	fmov d28, 3.0		// move 3.0 to register for multiplication operations

	fdiv x, d0, d28		// x = value / 3

cubeLoop:
	
	fmul y, x, x		// y = x^2
	fmul y, y, x  		// y = x^3

	fsub dy, y, d0		// dy - y - input
	
	fabs d26, dy		// get |dy|
	fmul d27, d0, errmax	// get input * max error
	fcmp d26, d27		// compare |dy| with input * 1e-10
	
	b.lt cubeEnd		// check if within error range

	fmul d25, x, x		// d25 = x^2
	fmul d25, d25, d28	// dy/dx = 3 * x^2

	fdiv d27, dy, d25	// d27 = dy / (dy/dx)

	fsub x, x, d27		// x = x - dy / (dy/dx)
	
	b cubeLoop		// go back to loop beginning

cubeEnd:			// jump here once cube root within error range
	fmov d0, x		// mov x to returned register
	ldp x29, x30, [sp], 16
	ret

