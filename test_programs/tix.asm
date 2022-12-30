	J	start
check	JGT	fail
	JLT	fail
	RSUB
pass	LDA	#0xBB
	J	halt
fail	LDA	#0xEE
	J	halt

start	LDX	#20
	LDA	#0x60
	LDS	#1

loop	TIX	#40
	JEQ	end
	ADDR	S, A
	J	loop

end	COMP	#0x69

	J	pass
halt	J	halt