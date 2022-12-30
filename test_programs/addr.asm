	J	start
check	JGT	fail
	JLT	fail
	RSUB
pass	LDA	#0xBB - 3 . Finish ok
	J	halt
fail	LDA	#0xEE . Finish fail
	J	halt

start	LDA	cookie
	COMP	cookie
	JSUB	check

	LDA	#cookie
	STA	indr
	COMP	#cookie
	JSUB	check


	LDA	@indr
	COMP	cookie
	JSUB	check

	+LDA	cookie
	COMP	cookie
	JSUB	check


	LDX	#3
	LDA	#0
	LDCH	table, X
	COMP	#0x4f
	JSUB	check

	+LDB	#far
	BASE	far
	LDA	far1
	COMP	#0xbe
	JSUB	check

	LDA	far3
	COMP	#0xce
	JSUB	check

	NOBASE

	J	pass
halt	J	halt

indr	RESW	1
cookie	WORD	0xdeadbe
table	BYTE	X'1f2f3f4f5f'

	ORG	0x5000
far	EQU	*
far1	WORD	0xbe
far2	WORD	0xde
far3	WORD	0xce