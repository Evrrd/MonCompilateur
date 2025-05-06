			# This code was produced by the CERI Compiler
	.data
	FormatString1:    .string "%llu"    # used by printf to display 64-bit unsigned integers
a:	.quad 0
b:	.quad 0
c:	.quad 0
d:	.quad 0
e:	.quad 0
f:	.quad 0
g:	.quad 0
	.text		# The following lines contain the program
	.globl main	# The main function must be visible from outside
main:			# The main function body :
	movq %rsp, %rbp	# Save the position of the stack's top
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function
