			# This code was produced by the CERI Compiler
	.data
	FormatString1:    .string "%llu"    # used by printf to display 64-bit unsigned integers
	FormatString2:    .string "%lf"    # used by printf to display 64-bit float
	FormatString3:    .string "%c"    # used by printf to display 64-bit char
a:	.quad 0
b:	.quad 0
e:	.double 0.0
f:	.double 0.0
	.text		# The following lines contain the program
	.globl main	# The main function must be visible from outside
main:			# The main function body :
	movq %rsp, %rbp	# Save the position of the stack's top
	push a
	push $3
	pop a
	push b
	push a
	push $2
	pop %rbx
	pop %rax
	addq	%rbx, %rax	# ADD
	push %rax
	pop b
	push f
	subq $8,%rsp			# allocate 8 bytes on stack's top
	movl	$0, (%rsp)	# Conversion of 2.5 (32 bit high part)
	movl	$1074003968, 4(%rsp)	# Conversion of 2.5 (32 bit low part)
	pop f
	push e
	push f
	subq $8,%rsp			# allocate 8 bytes on stack's top
	movl	$0, (%rsp)	# Conversion of 2 (32 bit high part)
	movl	$1073741824, 4(%rsp)	# Conversion of 2 (32 bit low part)
	fldl (%rsp) #empile first into float pile 
	addq $8, %rsp #depilement int pile 
	fldl (%rsp) #empile second into float pile 
	addq $8, %rsp #depilement int pile 
	fmulp %st(0),%st(1) # st0 * st1 , stockage dans st1 et depilement de st1 pcq p a la fin
	subq $8, %rsp    # Empilement d'une valeur non initialisée
	fstpl (%rsp)     # copie %st(0) au sommet de la pile générale et dépile la pile de registres %st(X)
	pop e
	push e
	movsd	(%rsp), %xmm0		# &stack top -> %xmm0
	subq	$16, %rsp		# allocation for 3 additional doubles
	movsd %xmm0, 8(%rsp)
	movq $FormatString2, %rdi	# "%lf\n"
	movq	$1, %rax
	call	printf
nop
	addq $24, %rsp			# pop nothing
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function
