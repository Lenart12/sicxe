loop	RD	#0xaa
	COMP	#0xff
	JEQ	halt
	WD	#1
	WD	#2
	WD	#0xab
	J	loop
halt	J	halt