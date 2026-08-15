/****************************************************************************/
/*
	VT.C  *dg*  08/2026 for MI-C compiler and CP/M
	
	Outputs a VT100 animation on console (CON:)
	
	The program can decode a simple compression mode where n consecutive
	identical characters are encoded as either 01h,xx or 02h,yy,zz.
	That is, a simple run-length encoding. Where xx represents 3 to 255
	repetitions and yy,zz represents 256 to 32767 repetitions
	(yy = low byte, zz = high byte).

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

int GetChr()
{
	return BDOS(0xFF, 6);
}

/****************************************************************************/

PutChr(ch)
	char ch;
{
	BDOS(ch, 6);
}

/****************************************************************************/

PutStr(s)
	char *s;
{
	while(*s)
		PutChr(*s++);
}		

/****************************************************************************/

main(argc, argv)
	int argc;
	char *argv[];
{
	char filename[13];
	FILE *fp;
	long totcnt;
	int i, cnt, c;
	BOOL decomp = FALSE;
	
	if (argc < 2)
	{
		PutStr("\r\nVT VT100 animation viewer *dg* 260815-01\r\n");
		PutStr(" usage: vt filename[.vt] [-d]  (use -d for decompression)\r\n");
		return;
	}

	if (index(argv[1], ".") == -1)
	{
		/* no extention */
		strncpy(filename, argv[1], 8);
		strcat(filename, ".vt");
	}
	else
	{
		/* with extention */
		strncpy(filename, argv[1], 12);
		filename[12] = '\0';
	}
	if (argc > 2 && argv[2][0]=='-' && tolower(argv[2][1])=='d')
	{
		decomp = TRUE;
	}

	fp = fopen(filename, "r");
	if (fp == NULL)
	{
		PutStr("\r\nfile ");
		PutStr(filename);
		PutStr(" not found\r\n");
		return;
	}

	PutStr("\033c");		/* VT100 reset terminal */
	Delay(1);
	PutStr("\033[?25l");	/* VT100 cursor off */
	PutStr("\033[?25l");	/* VT100 cursor off */
	PutStr("\033[2J");		/* VT100 clear screen */

	totcnt = 0;
	while(TRUE)
	{
		c = GetChr();
		if (c == 0x03) break; /* Ctrl-C */

		c = getc(fp);
		if (c == EOF) break;

		if (decomp)
		{
			if (c == 0x01)
			{
				cnt = getc(fp);
				c = getc(fp);
			}
			else if (c == 0x02)
			{
				cnt = getc(fp) + 256 * getc(fp);
				c = getc(fp);
			}
			else
			{
				cnt = 1;
			}
			if (c == 0x0A) putchar(0x0D);
			for (i = 0; i < cnt; i++)
			{
				putchar(c);
				totcnt++;
			}
		}
		else
		{
			if (c == 0x0A) putchar(0x0D);
			putchar(c);
			totcnt++;
		}
	}
	fclose(fp);

	PutStr("\033c");		/* VT100 reset terminal */
	Delay(1);
	PutStr("\033[?25h");	/* VT100 cursor on */
	PutStr("\033[?25h");	/* VT100 cursor on */

	printf("totcnt = %ld\r\n", totcnt);
}

/****************************************************************************/
