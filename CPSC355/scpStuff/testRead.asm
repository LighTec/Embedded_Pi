		define(fd_r, w19)
		define(nread_r, x20)
		define(buf_base_r, x21)
		//define(dval_r, d8)

		buf_size = 8
		alloc = -(16 + buf_size) & -16
		dealloc = -alloc
		buf_s = 16
		AT_FDCWD = -100

pn:		.string "input.bin" //filename
fmt1: 	.string "Error opening file: %s\nAborting.\n"
fmt2: 	.string "float = %f\n"

		.balign 4
		.global main 
main: 	stp 	x29, x30, [sp, alloc]!
		mov 	x29, sp

		mov		w0, AT_FDCWD 			// 1st arg (dirfd, -100 means current working directory)
		adrp 	x1, pn 					// 2nd arg (pathname, "output.bin")
		add 	x1, x1, :lo12:pn 		
		mov 	w2, 0 					// 3rd arg (flag, read-only)
		mov		w3, 0 					// 4th arg (not used)
		mov 	x8, 56 					// openat I/O request
		svc 	0 						// call system function
		mov		fd_r, w0 				// Record file descriptor

		// error check
		cmp 	fd_r, 0
		b.ge 	openok

		adrp 	x0, fmt1
		add 	x0, x0, :lo12:fmt1
		adrp 	x1, pn
		add  	x1, x1, :lo12:pn
		bl 		printf
		mov 	w0, -1
		b 		exit

openok: add 	buf_base_r, x29, buf_s

		// read floats from binary file one buffer at a time in a loop
top:	mov 	w0, fd_r 				// 1st arg (fd)
		mov 	x1, buf_base_r 			// 2nd arg (buf)
		mov 	w2, buf_size 			// 3rd arg (n)
		mov 	x8, 63 					// read I/O request
		svc		0 						// call system function
		mov 	nread_r, x0 			// record # of bytes actually read

		cmp 	nread_r, buf_size
		b.ne 	end

		// print out the read content
		adrp 	x0, fmt2
		add 	x0, x0, :lo12:fmt2
		ldr 	d0, [buf_base_r]
		fmov d21, d0			// for viewing in gdb

		//fmov	d0, dval_r
		bl  	printf

		b top

		// close the binary file
end: 	mov 	w0, fd_r
		mov		x8, 57
		svc 	0

		mov 	w0, 0
exit:	ldp 	x29, x30, [sp], dealloc
		ret
