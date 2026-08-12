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

#define MAXDRIVES 4

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

int ReadDir(list, drvcnt, drvlst)
	char **list;
	int drvcnt;
	char *drvlst;
{
	stfcb *mask, *dir;
	int result, i, d, cnt;
	char name[15], *pn, *p;
	BOOL first;
	
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
	strncpy(mask->fn, "????????VT?", 11); /* copy without '\0' */
	cnt = 0;
	for (d = 0; d < drvcnt; d++)
	{
		mask->dr = drvlst[d]; /* 0=current, 1=A, 2=B, ... */
		first = TRUE;
		while(TRUE)
		{
			BDOS(dir, 26);					/* set DMA address */
			if (first)
			{
				result = BDOS(mask, 17); 	/* get first dir entry */
				first = FALSE;
			}
			else
				result = BDOS(mask, 18); 	/* get next dir entry */
			if (result == 255) break;

			p = (char *)dir + result * 32 + 1;
			pn = name;
			if (drvlst[d] > 0)
			{
				*pn++ = drvlst[d] - 1 + 'A';
				*pn++ = ':';
			}
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
				strncpy(p, name, 14); /* d:xxxxxxxx.yyy = 14 chars */
				if (strlen(name) > 13)
					p[14] = '\0';
				list[cnt] = p;
			}
		
			cnt++;
		}
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
	char drvlst[MAXDRIVES];
	int filcnt, plycnt, drvcnt, i, f, ch;
	BOOL fnd;

	i = 0;
	if (argc < 2)
	{
		PutStr("VTSHOW *dg* 260812-01\r\n");
		PutStr(" usage: vtshow [<d> <d>...] <random seed>  (1..32748)\r\n");
		PutStr("  e.g.: vtshow a b 100\r\n");
		return;
	}
	drvcnt = 0;
	for (i = 0; i< MAXDRIVES; i++)
	{
		if (i + 2 > argc) break;
		if (toupper(argv[i + 1][0]) >= 'A' && toupper(argv[i + 1][0]) <= 'P')
		{
			drvlst[drvcnt] = argv[i + 1][0] - 'A' + 1;
			drvcnt++;
		}
	}
	if (drvcnt == 0)
	{
		drvlst[0] = 0;
		drvcnt++;
	}

	i = atoi(argv[argc - 1]);
	if (i < 0) i = -i;
	RndSeed(i);

	/* first read to get file count */
	filcnt = ReadDir(NULL, drvcnt, drvlst);
	if (filcnt == 0)
	{
		PutStr("no vt files\r\n");
		return;
	}
	list = calloc(filcnt, 2);
	if (list == NULL)
	{
		PutStr("calloc error\r\n");
		return;
	}
	/* second read to get list */
	filcnt = ReadDir(list, drvcnt, drvlst);
	if (filcnt == 0)
	{
		PutStr("error reading vt files\r\n");
		return;
	}
	
	shown = calloc(filcnt, 1);
	if (shown == NULL)
	{
		PutStr("calloc error\r\n");
		return;
	}

	while(TRUE)
	{
		for (f = 0; f < filcnt; f++)
			shown[f] = 0;

		plycnt = 0;
		while(plycnt < filcnt)
		{
			ch = GetChr();
			if (ch == 0x03) break; /* Ctrl-C */

			fnd = FALSE;
			for (i = 0; i < 5; i++)
			{
				f = RndNext(0, filcnt);
				if (!shown[f])
				{
					fnd = TRUE;
					break;
				}
			}
			if (!fnd)
			{	/* not found */
				for (f = 0; f < filcnt; f++)
				{
					if (!shown[f])
					{
						fnd = TRUE;
						break;
					}
				}
				if (!fnd)
				{
					/* this should not happen */
					break;
				}
			}
			
			plycnt++;
			shown[f] = 1;
			
			ch = ShowFile(list[f]);
			if (ch == 0X03) break;
			
			Delay(1);
		}
		if (ch == 0X03) break;
	}

	PutStr("\033c");		/* VT100 reset terminal */
	Delay(1);
	PutStr("\033[?25h");	/* VT100 cursor on */
	PutStr("\033[?25h");	/* VT100 cursor on */
}

/****************************************************************************/
