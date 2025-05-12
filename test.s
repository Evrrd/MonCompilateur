			# This code was produced by the CERI Compiler
	.data
	FormatString1:    .string "%llu"    # used by printf to display 64-bit unsigned integers
	FormatString2:    .string "%f"    # used by printf to display 64-bit float
	FormatString3:    .string "%c"    # used by printf to display 64-bit char
a:	.quad 0
b:	.quad 0
c:	.quad 0
d:	.double 0.0
e:	.double 0.0
f:	.double 0.0
g:	.double 0.0
	.text		# The following lines contain the program
	.globl main	# The main function must be visible from outside
main:			# The main function body :
	movq %rsp, %rbp	# Save the position of the stack's top
	push a
	push $3
	push f
	fld $0.6
	push e
	push f
	fld $4.1
	fldl (%rsp) #empile first into float pile 
	addq $8, %rsp #depilement int pile 
	fldl (%rsp) #empile second into float pile 
	faddpl	%st(0),%st(1) # st0 + st1 , stockage dans st1 et depilement de st1 pcq p a la fin
	subq $8, %rsp    # Empilement d'une valeur non initialisée
	fstpl (%rsp)     # copie %st(0) au sommet de la pile générale et dépile la pile de registres %st(X)
	push e
pop %rdx                     # The value to be displayed
movq $FormatString1, %rsi    # %f

movl    $1, %edi
movl    $0, %eax
call    __printf_chk@PLT
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function
