/****************************************************************************/
/*
	VTSHOW.C  *dg*  08/2026 for MI-C compiler and CP/M
	
	Random Show of VT100 animations
	
*/
/****************************************************************************/

#include "stdio.h"

#define FALSE 0
#define TRUE 1
#define WORD unsigned int
#define BOOL int

typedef struct {
	char dr;		/* Drive code (0 = default, 1 = A:, etc.) */
	char fn[8];		/* File name (padded with spaces) */
	char ft[3];		/* File type/extension (padded) */
	char ex;		/* Extent number */
	char s1;		/* Reserved */
	char s2;		/* Extent high byte / block count */
	char rc;		/* Record count in current extent */
	char al[16];	/* Disk allocation map (managed by CP/M) */
	char cr;		/* Current record to read/write */
} stfcb;

long rndstate;

/****************************************************************************/
/* aprox. 1 s delay */

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
/*
 * 16-bit PRNG:
 * Multiplicative congruential generator modulo 32749.
 *
 * state = (23 * state) mod 32749
 *
 * 32749 is prime and 23 is a primitive root modulo 32749, so all seeds
 * from 1 to 32748 produce a cycle with period 32748.
 */

int RndSeed(seed)
	int seed;
{
    seed %= 32749;

    if (seed <= 0)
        seed += 32748;

	rndstate = seed;
    return rndstate;
}

#define RND_A 23
#define RND_M 32749
#define RND_Q 1423  /* m / a */
#define RND_R 20 /* m % a */

/* random number from "from" to "to-1" */
int RndNext(from, to)
	int from, to;
{
	int hi, lo, t;
	
    hi = rndstate / RND_Q;
    lo = rndstate % RND_Q;
    rndstate = RND_A * lo - RND_Q * hi;
    if (rndstate <= 0)
        rndstate += RND_M;

	return rndstate * (to - from) / RND_M + from;
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

/*
Dump(ptr)
	char *ptr;
{
	int i;

	printf("ptr %04X: ", ptr);
	for (i = 0; i<sizeof(stfcb); i++)
	{
		printf("%02X ", ptr[i]);
	}
	PutStr("\r\n");
}
*/

/****************************************************************************/

int ReadDir(list)
	char **list;
{
	stfcb *mask, *dir;
	int result, i, cnt;
	char name[13], *pn, *p;
	
	mask = calloc(1, sizeof(stfcb));
	if (mask == NULL)
	{
		PutStr("calloc error\r\n");
		return;
	}
	dir = calloc(1, 128); /* 128 byte for one directory entry */
	if (dir == NULL)
	{
		PutStr("calloc error\r\n");
		return;
	}

	p = mask;
	for(i = 0; i<sizeof(stfcb); i++)
		p[i] = 0;

	/*mask->dr = 6;*/ /* F: */
	mask->dr = 0; /* current */
	strncpy(mask->fn, "????????VT?", 11); /* copy without '\0' */

	cnt = 0;
	while(TRUE)
	{
		BDOS(dir, 26);	/* set DMA address */
		if (cnt == 0)
			result = BDOS(mask, 17); 	/* get first dir entry */
		else
			result = BDOS(mask, 18); 	/* get next dir entry */
		if (result == 255) break;

		p = (char *)dir + result * 32 + 1;
		pn = name;
		for (i=0; i<11; i++)
		{
			if (p[i] == ' ') continue;
			if (i==8) *pn++ = '.';
			*pn++ = p[i];
		}
		*pn = '\0';
	
		if (list != NULL)
		{
			p = calloc(strlen(name) + 1, 1);
			if (p == NULL) break;
			strncpy(p, name, 12);
			if (strlen(name) > 11)
				p[12] = '\0';
			list[cnt] = p;
		}
	
		cnt++;
	}
	
	return cnt;
}

/****************************************************************************/

ShowFile(filename)
	char *filename;
{
	FILE *fp;
	int cnt, ret;
	int c, f;
	
	fp = fopen(filename, "ra");
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
	PutStr("\033[2J");		/* VT100 clear screen */

	cnt = 0;
	ret = 0;
	while(TRUE)
	{
		cnt++;
		
		c = GetChr();
		if (c == 0x03) 
		{
			ret = c;
			break; /* Ctrl-C */
		}
		if (c != 0x00) 
		{
			ret = 0;
			break; /* any key */
		}
		
		c = getc(fp);
		if (c == EOF) break;

		if (c == 0x0A) putchar(0x0D);
		putchar(c);
	}
	fclose(fp);

	return ret;
}

/****************************************************************************/

main(argc, argv)
	int argc;
	char *argv[];
{
	char **list, *shown;
	int filCnt, playCnt, i, f, ch;

	filCnt = ReadDir(NULL);
	if (filCnt == 0)
	{
		PutStr("no vt files\r\n");
		return;
	}
	list = calloc(filCnt, 2);
	if (list == NULL)
	{
		PutStr("calloc error\r\n");
		return;
	}
	filCnt = ReadDir(list);
	if (filCnt == 0)
	{
		PutStr("error reading vt files\r\n");
		return;
	}
	shown = calloc(filCnt, 1);
	if (shown == NULL)
	{
		PutStr("calloc error\r\n");
		return;
	}

	RndSeed(1246);

	while(TRUE)
	{
		for (f = 0; f < filCnt; f++)
			shown[f] = 0;

		playCnt = 0;
		while(playCnt < filCnt)
		{
			ch = GetChr();
			if (ch == 0x03) break; /* Ctrl-C */
			
			for (i = 0; i < 5; i++)
			{
				f = RndNext(0, filCnt);
				if (!shown[f]) break;
				f = 0;
			}
			if (f == 0)
			{	/* not found */
				for (f = 0; f < filCnt; f++)
				{
					if (!shown[f]) break;
					f = 0;
				}
				if (f == 0)
				{
					/* this should not happen */
					break;
				}
			}
			
			playCnt++;
			shown[f] = 1;
			
			ch = ShowFile(list[f]);
			if (ch == 0X03) break;
			
			Delay(1);
		}
		if (ch == 0X03) break;
	}

	return;

	PutStr("\033c");		/* VT100 reset terminal */
	Delay(1);
	PutStr("\033[?25h");	/* VT100 cursor on */
	return;
	
	
#if FALSE
	
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

#endif
	
}

/****************************************************************************/
