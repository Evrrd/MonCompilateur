			# This code was produced by the CERI Compiler
	.data
	FormatString1:    .string "%llu"    # used by printf to display 64-bit unsigned integers
	FormatString2:    .string "%lf"    # used by printf to display 64-bit float
	FormatString3:    .string "%c"    # used by printf to display 64-bit char
TrueString:	.string "TRUE"	# used by printf to display the boolean value TRUE
FalseString:	.string "FALSE"	# used by printf to display the boolean value FALSE
a:	.quad 0
b:	.quad 0
f:	.quad 0
	.align 8
	.text		# The following lines contain the program
	.globl main	# The main function must be visible from outside
main:			# The main function body :
	movq %rsp, %rbp	# Save the position of the stack's top
	push a
	push $3
	pop a
	addq $8, %rsp			# pop nothing
	push b
	push $0
	pop b
	addq $8, %rsp			# pop nothing
	push f
	push $2
	pop f
	addq $8, %rsp			# pop nothing
	push $5
	push $5
	pop %rbx
	pop %rax
	addq	%rbx, %rax	# ADD
	push %rax
Loop0:
	pop %rax 	 #get the result of expression
	cmpq %rax, f
	push %rax
	jae EndLoop0 #jump if equal we out of the loop
	push f
	pop %rsi	# The value to be displayed
	movq $FormatString1, %rdi	# "%llu\n"
	movl	$0, %eax
	call	printf@PLT
	addq $1, f 	 #adding one to f
	jmp Loop0
EndLoop0:
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function
