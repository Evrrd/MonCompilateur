			# This code was produced by the CERI Compiler
	.data
	FormatString1:    .string "%llu"    # used by printf to display 64-bit unsigned integers
	.align 8
a:	.quad 0
	.text		# The following lines contain the program
	.globl main	# The main function must be visible from outside
main:			# The main function body :
	movq %rsp, %rbp	# Save the position of the stack's top
	push $5
	push $6
	pop %rbx
	pop %rax
	addq	%rbx, %rax	# ADD
	push %rax
	pop a
	push a
	push $40
	pop %rax
	pop %rbx
	cmpq %rax, %rbx
	ja Vrai1	# If above
	push $0		# False
	jmp Suite1
Vrai1:	push $0xFFFFFFFFFFFFFFFF		# True
Suite1:
	push $1
pop %rdx                     # The value to be displayed
movq $FormatString1, %rsi    # %llu

movl    $1, %edi
movl    $0, %eax
call    __printf_chk@PLT
	push $0
pop %rdx                     # The value to be displayed
movq $FormatString1, %rsi    # %llu

movl    $1, %edi
movl    $0, %eax
call    __printf_chk@PLT
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function
