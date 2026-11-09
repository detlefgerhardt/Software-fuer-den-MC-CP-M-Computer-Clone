/* MONIO.C *dg* 08.10.2026 */
/* monio.c combines all conio funtions by using Monitor functions */

#ASM
MONIN	EQU 0F003h
MONOUT	EQU 0F009h
MONSTS	EQU 0F012h
#ENDASM


/* mputch() using Monitor function MONOUT */
int mputch(c)
	int c;
{
#ASM
	LD IX,2			; RET address
	ADD IX,SP
	LD C,(IX+0)		; character code
	PUSH BC
	CALL MONOUT
	POP HL			; ret char code
#ENDASM
}

/* mgetch() using nonblocking Monitor MONIN */
int mgetch()
{
#ASM
	CALL MONIN
	LD L,A			; char code in A
	LD H,0
#ENDASM
}

/* mkbhit() using nonblocking Monitor function MONSTS */
/* return 1 if character from console is available and 0 if not */
int mkbhit()
{
#ASM
	CALL MONSTS
	AND A			; status in A
	JR Z,mkbh1		; no char available
	; char avaiable
	LD HL,1
	RET
mkbh1:
	LD HL,0

#ENDASM
}

/* outputs a string to console */
mputs(s)
	char *s;
{
	while(*s)
		mputch(*s++);
}

/* end of file */
