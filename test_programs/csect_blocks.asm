. Start section
. prog1	START	0
	EXTREF	first2
	EXTDEF	var1, var2, var3
	USE
first1	LDA	#0
	LDB	#0
	LDS	#0
	J	second
	USE	BLK1
second	LDA	var1
	LDB	var2
	LDS	var3
	J	third
	USE	BLK2
third	LDA	#2
	LDB	#2
	LDS	#2
	J	fagain
	USE
fagain	LDA	=1
	LDA	=1
	LDA	=1
	+J	first2
	USE	PDATA
var1	WORD	0x185
var2	WORD	0x186
var3	WORD	0x187
	LTORG

. Section 2
sec2	CSECT
	EXTREF	var1, var2, var3
	EXTDEF	first2
	USE
first2	LDA	#0
	LDB	#0
	LDS	#0
	J	second
	USE	BLK1
second	+LDA	var1
	+LDB	var2
	+LDS	var3
	J	third
	USE	BLK2
third	LDA	=2
	LDB	=2
	LDS	=2
	J	fagain
	USE
fagain	LDA	=1
	LDA	=1
	LDA	=1
halt	J	halt
	LTORG
	END	first1