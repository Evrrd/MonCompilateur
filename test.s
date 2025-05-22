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
	push $8
	pop %rax 	 #get the result of expression
	push a
Case0:
	pop %rbx
	cmp %rax, %rbx
	jne Case1
	push $100
	pop %rsi	# The value to be displayed
	movq $FormatString1, %rdi	# "%llu\n"
	movl	$0, %eax
	call	printf@PLT
Case1:
	push c
	pop %rbx
	cmp %rax, %rbx
	jne Case2
	push $200
	pop %rsi	# The value to be displayed
	movq $FormatString1, %rdi	# "%llu\n"
	movl	$0, %eax
	call	printf@PLT
Case2:
	jmp EndCase3
EndCase3:
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function
