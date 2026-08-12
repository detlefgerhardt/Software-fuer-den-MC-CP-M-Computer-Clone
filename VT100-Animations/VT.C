/****************************************************************************/
/*
	VT.C  *dg*  08/2026 for MI-C compiler and CP/M
	
	Show a VT100 animation
	
*/
/****************************************************************************/

#include "stdio.h"

#define FALSE 0
#define TRUE 1
#define WORD unsigned int
#define BOOL int

/****************************************************************************/

int BD_CoIn()
{
	return BDOS(0xFF, 6);
}

/****************************************************************************/

BD_CoOut(ch)
	char ch;
{
	BDOS(ch, 6);
}

/****************************************************************************/

int GetChr()
{
	return BD_CoIn();
}

/****************************************************************************/

PutChr(ch)
	char ch;
{
	BD_CoOut(ch);
}

/****************************************************************************/

PutStr(s)
	char *s;
{
	while(*s)
		/*BD_CoOut(*s++);*/
		putchar(*s++);
}		

/****************************************************************************/

main(argc, argv)
	int argc;
	char *argv[];
{
	char filename[13];
	FILE *fp;
	int cnt;
	int c;
	
	if (argc < 2)
	{
		PutStr("\r\nVT VT100 animation viewer *dg* 260812-01\r\n");
		PutStr(" usage: vt filename[.vt]\r\n");
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


	fp = fopen(filename, "ra");
	if (fp == NULL)
	{
		PutStr("\r\nfile ");
		PutStr(filename);
		PutStr(" not found\r\n");
		return;
	}

	/*PutStr("\033c");*/		/* VT100 reset terminal */
	PutStr("\033[?25l");	/* VT100 cursor off */
	PutStr("\033[2J");		/* VT100 clear screen */

	cnt = 0;
	while(TRUE)
	{
		cnt++;
		
		c = GetChr();
		if (c == 0x03) break; /* Ctrl-C */
		
		c = getc(fp);
		
		if (c == EOF) break;

		if (c == 0x0A) putchar(0x0D);
		putchar(c);
	}
	fclose(fp);

	/*PutStr("\033c");*/		/* VT100 reset terminal */
	PutStr("\033[?25h");	/* VT100 cursor on */
}

/****************************************************************************/
