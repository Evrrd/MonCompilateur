			# This code was produced by the CERI Compiler
	.data
	FormatString1:    .string "%llu"    # used by printf to display 64-bit unsigned integers
	FormatString2:    .string "%lf"    # used by printf to display 64-bit float
	FormatString3:    .string "%c"    # used by printf to display 64-bit char
TrueString:	.string "TRUE"	# used by printf to display the boolean value TRUE
FalseString:	.string "FALSE"	# used by printf to display the boolean value FALSE
a:	.quad 0
b:	.quad 0
c:	.quad 0
d:	.quad 0
	.align 8
	.text		# The following lines contain the program
	.globl main	# The main function must be visible from outside
main:			# The main function body :
	movq %rsp, %rbp	# Save the position of the stack's top
	push a
	push $1
	pop a
	addq $8, %rsp			# pop nothing
	push b
	push $4
	pop b
	addq $8, %rsp			# pop nothing
	push c
	push $8
	pop c
	addq $8, %rsp			# pop nothing
	push d
	push a
	push b
	pop %rax
	pop %rbx
	cmpq %rax, %rbx
	jb Vrai2	# If below
	push $0		# False
	jmp Suite2
Vrai2:	push $0xFFFFFFFFFFFFFFFF		# True
Suite2:
	pop d
	addq $8, %rsp			# pop nothing
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function
