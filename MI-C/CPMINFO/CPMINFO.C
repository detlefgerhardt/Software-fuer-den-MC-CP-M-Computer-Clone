/****************************************************************************/
/*
	CPMINFO.C  *dg*  11/2026 for MI-C compiler and CP/M
*/
/****************************************************************************/
/* NFORM version history

 08.11.2026 *dg* First version
*/

#include "stdio.h"

#define FALSE 0
#define TRUE 1
#define WORD unsigned int
#define BOOL int

/*
#ASM
	EXTERNAL STKPTR
#ENDASM
*/

/****************************************************************************/

/*
getstk()
{
#ASM
	LD HL,(STKPTR)
	RET
#ENDASM
}
*/

main(argc, argv)
	int argc;
	char *argv[];
{
	unsigned cpm;
	unsigned ccpsize;
	unsigned bios;
	unsigned bdos;
	unsigned bdossize;
	unsigned stk;
	char hexbuf[5];
	
	cputs("\r\nCPMINFO *dg* 261108-01\r\n\n");

	ccpsize = 0x0800;			/* size of CCP (CP/M command processor) */

	bdos = getint(0x0006) - 6;	/* 0x0006 = BDOS entry address */
	bios = getint(0x0001) - 3;	/* 0x0001 = warmstart adress */
	cpm = bdos - ccpsize;
	bdossize = bios - bdos;

	cprintf("CPM  start: %04Xh  (CCP  size = %04Xh)\r\n", cpm, ccpsize);
	cprintf("BDOS start: %04Xh  (BDOS size = %04Xh)\r\n", bdos, bdossize);
	cprintf("BIOS start: %04Xh\r\n", bios);

	/*
	stk = getstk();
	cprintf("STK  start: %04Xh\r\n", stk);
	*/
	
	EXIT();
}

/****************************************************************************/
