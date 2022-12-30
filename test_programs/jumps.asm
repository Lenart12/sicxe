	J	start
pass	LDA	=0xBB
	J	halt
fail	LDA	=0xEE
	J	halt

start	LDA	=0
	COMP	=5
	JLT	next
	J	fail
next	LDA	=5
	COMP	=0
	JGT	next2
	J	fail
next2	LDA	=-5
	COMP	=0
	JLT	next3
	J	fail
next3	LDA	=0
	COMP	=-5
	JGT	next4
	J	fail
next4	LDA	=-20
	COMP	=-20
	JEQ	pass
	J	fail


halt	J	halt