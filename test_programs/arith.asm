	J	start
check	JGT	fail
	JLT	fail
	RSUB
pass	LDA	#0xBB
	J	halt
fail	LDA	#0xEE
	J	halt

start	LDA	=0
	SUB	=5
	COMP	=-5
	JSUB	check

	LDA	=-5
	ADD	=10
	COMP	=5
	JSUB	check


	LDA	=-1
	MUL	=10
	COMP	=-10
	JSUB	check


	LDA	=13
	DIV	=5
	COMP	=2
	JSUB	check


	. LDA	=0xf0f0f0
	. SHIFTL	A, 2
	. COMP	=0xc3c3c3
	. JGT	fail
	. JLT	fail

	. LDA	=0xf0f0f0
	. SHIFTR	A, 6
	. COMP	=0xc3c3c3
	. JGT	fail
	. JLT	fail

	LDA	=5
	LDT	=10
	ADDR	A, T
	LDS	=15
	COMPR	T, S
	JSUB	check

	RMO	T, A
	COMP	=15
	JSUB	check

	J	pass


halt	J	halt