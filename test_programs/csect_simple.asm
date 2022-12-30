. Section [default] imports do_add
prog1	START	0
	EXTREF	do_add
	+JSUB	do_add
halt	J	halt
	END	prog1

. Section [sect1] exports var1, var2
sect1	CSECT
	EXTDEF	var1, var2
var1	WORD	1
var2	WORD	2

. Section [sect2] exports do_add imports var1, var2
sect2	CSECT
	EXTDEF	do_add
	EXTREF	var1, var2
do_add	+LDA	var1
	+ADD	var2
	RSUB
