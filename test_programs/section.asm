prog	START	1000
check	JGT	fail
	JLT	fail
	RSUB
pass	LDA	#0xBB
	J	halt
fail	LDA	#0xEE
	J	halt

start	+LDA	=0xdeadbe
	COMP	=0xdeadbe
	JSUB	check

	J	pass
halt	J	halt
	END	start