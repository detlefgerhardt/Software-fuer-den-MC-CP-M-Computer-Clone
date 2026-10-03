/****************************************************************************/
/*
	SERIAL.C  *dg*  10/2026 for MI-C compiler and CP/M
	
	Direct serial output to SIO port
	
*/
/****************************************************************************/

#include "stdio.h"
#include "conio.h"

#define FALSE 0
#define TRUE 1
#define WORD unsigned int
#define BOOL int

/****************************************************************************/

main()
{
	char ch;
	
	cprintf("Hello world!\r\n");

	SA_INIT();
	SA_OUT('a');
	SA_Out('b');
	SA_Out('c');
	SA_Out('\r');
	SA_Out('\n');

	while(TRUE)
	{
		if (kbhit()) break;
		if (SA_ST() != 0)
		{
			ch = SA_IN();
			SA_Out(ch);
			if (ch == 0x03) return;
		}
	}
}

/****************************************************************************/
