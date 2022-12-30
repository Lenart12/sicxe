. Section [sect2] exports do_add imports var1, var2
sect2	CSECT
	EXTDEF	do_add
	EXTREF	var1, var2
do_add	+LDA	var1
	+ADD	var2
	RSUB