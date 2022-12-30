. Section [default] imports do_add
prog1	START	0
	EXTREF	do_add
first	+JSUB	do_add
halt	J	halt
	END	first