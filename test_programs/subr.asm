	J	start
check	JGT	fail
	JLT	fail
	RSUB
pass	LDA	#0xBB
	J	halt
fail	LDA	#0xEE
	J	halt

start	LDA	=5
	COMP	=5
	JSUB	check

	J	pass
halt	J	halt