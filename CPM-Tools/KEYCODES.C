/****************************************************************************/
/*
	KEYCODES.C  *dg*  08/2026 for MI-C compiler and CP/M
	
	Read and show keycodes from console

*/
/****************************************************************************/

#include "stdio.h"

#define FALSE 0
#define TRUE 1
#define WORD unsigned int
#define BOOL int

/****************************************************************************/
/* a delay of very roughly 1 second, depends on CPU clock
 */

Delay(s)
	int s;
{
	long cnt;
	int i;
	
	for (i = 0; i<s; i++)
	{
		for (cnt = 0; cnt < 15000; cnt++);
	}
}

/****************************************************************************/

main()
{
	int ch;
	long cnt;
	int keycnt;
	char keys[10];

	while(TRUE)
	{
		cnt = 0;
		keycnt = 0;
		WHILE(TRUE)
		{
			cnt++;
			if (cnt > 1000) break;
			
			if (kbhit())
			{
				ch = getch();
				if (ch == 0x03) EXIT();
				keys[keycnt] = ch;
				cnt = 0;
				keycnt++;
				if (keycnt >= 10) break;
			}
		}

		if (keycnt > 0)
		{
			printf("key:");
			for (cnt = 0; cnt < keycnt; cnt++)
				printf(" %02X", keys[cnt]);
			printf("\r\n");
		}
	}
}

/****************************************************************************/
