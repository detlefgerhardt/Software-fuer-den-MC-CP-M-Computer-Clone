/****************************************************************************/
/*
	LIBTEST.C  *dg*  08/2026 for MI-C compiler and CP/M
	
*/
/****************************************************************************/

#include "stdio.h"
/*#include "conio.h"*/
/*#include "dglib.h"*/
#include "vt100.h"

#define FALSE 0
#define TRUE 1
#define WORD unsigned int
#define BOOL int

char xprbuf[80];

char buf1[20];
char buf2[20];

/****************************************************************************/

stritst(s1, s2)
	char *s1, *s2;
{
	int i;
	
	i = stricmp(s1, s2);
	cprintf("stricmp '%s' '%s' = %d\r\n", s1, s2, i);
}

/****************************************************************************/

strintst(s1, s2, n)
	char *s1, *s2;
	int n;
{
	int i;
	
	i = strincmp(s1, s2, n);
	cprintf("stricmp '%s' '%s' %d = %d\r\n", s1, s2, n, i);
}

/****************************************************************************/

main(argc, argv)
	int argc;
	char *argv[];
{
	int c;
	int i, x, y;
	long l;
	char *s, *s1, *s2;
	char *chrbuf = 0x0005;
	char buf[9];
	
	/*
	memset(buf1, 1, 20);
	memcpy(buf2, buf1, 20);

	for (i = 0; i< 20; i++)
		printf("%d ", buf2[i]);
	printf("\r\n");

	cputs("test\r\n");
	*/

	/*
	while(TRUE)
	{
		i = getch();
		if (i==3) break;
		printf("%d ", i);
	}
	*/

	/*
	ungetch('x');
	cputs("input char\r\n");
	while(!kbhit());
	cputs("kbhit()\r\n");
	c = getch();
	putch(c);
	*/

	/*
	putch('a');
	putch('b');
	putch('\r');
	putch('\n');

	printf("%02X %02X %02X\r\n", chrbuf[0],chrbuf[1],chrbuf[2]);
	`*/

	vt_norm();
	vt_goff();

	cputs("\r\nattributes:\r\n");
	vt_puts("\003emph\001\r\n");
	vt_puts("\004unterscore\001\r\n");
	vt_puts("\005blink\001\r\n");
	/*vt_puts("\006hidden\001\r\n");*/
	vt_puts("\007invers\001\r\n");

	cputs("\r\ngraphic characters:\r\n");
	vt_puts("\010");
	vt_puts("\154 \161 \167 \153\r\n");
	vt_puts("\164 \170 \156 \165\r\n");
	vt_puts("\155 \161 \166 \152\r\n");
	vt_puts("\011");

	cputs("\r\ncolors:\r\n");
	for (y = 0; y < 8; y++)
	{
		vt_backcol(y);
		for (x = 0; x < 8; x++)
		{
			vt_forecol(x);
			cputs("A");
		}
		vt_forecol(VT_DEF);
		vt_backcol(VT_DEF);
		cputs("\r\n");
	}

	stritst("ABC", "ABC");
	stritst("ABC", "abc");
	stritst("ABC", "abd");
	stritst("ABd", "abc");
	stritst("ABC", "abcd");
	stritst("ABCd", "abc");

	strintst("ABC", "ABC", 3);
	strintst("ABC", "abc", 3);
	strintst("ABC", "abd", 3);
	strintst("ABd", "abc", 3);
	strintst("ABC", "abcd", 3);
	strintst("ABCd", "abc", 3);


	cputs("\r\ntest kbhit()/getch(), abort with CTRL-C:");
	cputs("\r\ninput: ");
	while(TRUE)
	{
		if (kbhit())
		{
			cputs("[hit] ");
			c = getch();
			cprintf("%c %02X", c, c);
			if (c == 0x03) break;
			cputs("\r\ninput: ");
		}
	}

	
#if FALSE	
	ctohex(193, buf);
	printf("%d %s\r\n", 193, buf);
	utohex(1024*5+14, buf);
	printf("%u %s\r\n", 1024*5+14, buf);
	ltohex(1024*5+14, buf);
	printf("%u %s\r\n", 1024*5+14, buf);

	ltohex(12345678L, buf);
	printf("%lu %s\r\n", 12345678L, buf);
	ltohex(92345678L, buf);
	printf("%lu %s\r\n", 92345678L, buf);

	l = -1;
	/*l = 0x7FFFFFFFFL;*/
	ltohex(l, buf);
	printf("%ld %s\r\n", l, buf);
	
	s = "abcDEF*";
	s = strlwr(s);
	printf("%s\r\n", s);
	s = "ABCdef*";
	s = strupr(s);
	printf("%s\r\n", s);
#endif	

	EXIT(0);
}

/****************************************************************************/
