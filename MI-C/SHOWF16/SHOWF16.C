/****************************************************************************/
/*
	SHOWF16.C  *dg*  10/2026 for MI-C compiler and MC CP/M clone
	
	Show FAT16 formated compact flash (without MBR)
*/
/****************************************************************************/
/* SHOWF16 version history

 29.10.2026 *dg* - First version (from COPYCF and SHOWF12)
 
 */

#include "stdio.h"
#include "phydrv.h"

#define FALSE 0
#define TRUE 1
#define WORD unsigned int
#define BOOL int

#define CTRLC 3
#define CR 13

#define IOADDR 0x18

#define VOLCNT 4 /* number of volumes */

#define SECSIZ 512
char *secbuf;

typedef struct
{
	char name[12]; /* not null-terminated */
	long size;
	char attr;
	unsigned clust;
} DirEnt;

DirEnt *DirList;
#define DIRMAX 128
#define DIRLEN (DIRMAX/32)
int dirstr;	/* start sector */
int dircnt;

#define FATLEN 1
int fatstr;
char *fat;

extern long getlng();

/****************************************************************************/
/* BDOS init disk system */

InitDisks()
{
	BDOS(0, 0x0D);
}

/****************************************************************************/
/* dump memory from address p, count bytes
   use dspadr as the displayed address
*/

#if FALSE

dump(p, count, dspadr)
	char *p;
	int count;
	unsigned dspadr;
{
	int cnt,col,ch;
	char s[5];
	int cols = 16;

	if (dspadr == 0XFFFF) dspadr = p;

	cnt = 0;
	while(cnt < count)
	{
		utohex(dspadr, s);
		cputs(s);
		for (col = 0; col < cols; col++)
		{
			putch(' ');
			if (cnt + col < count)
			{
				ctohex(*(p+col), s);
				cputs(s);
			}
			else
				cputs("  ");
			if (col == 7)
				putch(' ');
		}
		putch(' ');
		for (col = 0; col < cols; col++)
		{
			if (cnt + col >= count) break;
			ch = *(p+col);
			if (ch >= 0x20 && ch <= 0x7E)
				putch(ch);
			else
				putch('.');
			if (col == 7)
				putch(' ');
		}
		cputs("\r\n");
		cnt += cols;
		p += cols;
		dspadr += cols;
	}
}

#endif

/****************************************************************************/

int WaitCr()
{
	int ch;
	
	while(TRUE)
	{
		ch = getch();
		if (ch == CTRLC || ch == CR) return ch; /* Ctrl-C or ENTER */
	}
}

/****************************************************************************/
/* returns TRUE ich CTRL-C was hit, nonblocking */

BOOL CtrlC()
{
	int ch;
	
	if (kbhit())
	{
		ch = getch();
		if (ch == CTRLC) 
		{
			cprintf("\r\nCtrl-C detected. Aborted!\r\n");
			return TRUE;
		}
	}
	return FALSE;
}

/****************************************************************************/

long memlong(p)
	char *p;
{
	
}

/****************************************************************************/

ShowF16()
{
	BOOL ok;
	char oem[8+1];
	char label[11+1];
	char systyp[8+1];
	int secsiz, clucnt, clusiz, media, reserv, fatcnt, fatsiz;
	int maxdir, seccnt, extbpb;
	
	

	rd_lba(secbuf, IOADDR, 0L, TRUE);
	hexdmp(secbuf, SECSIZ, 0x0000);
	
	strncpy(oem, secbuf + 3, 8);
	oem[8] = 0;
	cprintf("oem = '%s'\r\n", oem);

	secsiz = getint(secbuf + 0x0B);
	cprintf("secsiz = %d\r\n", secsiz);

	clucnt = secbuf[0x0D];
	clusiz = clucnt * secsiz;
	cprintf("clucnt = %d\r\n", clucnt);
	cprintf("clusiz = %d\r\n", clusiz);

	reserv = getint(secbuf + 0x0E);
	cprintf("reserv = %d\r\n", reserv);
	
	fatcnt = secbuf[0x10];
	cprintf("fatcnt = %d\r\n", fatcnt);
	
	maxdir = getint(secbuf + 0x11);
	cprintf("maxdir = %d\r\n", maxdir);

	seccnt = getint(secbuf + 0x13);
	cprintf("seccnt = %d\r\n", seccnt);

	media = secbuf[0x15];
	cprintf("media = %02X\r\n", media);

	fatsiz = getint(secbuf + 0x16);
	cprintf("fatsiz = %d\r\n", fatsiz);

	extbpb = secbuf[0x26];
	cprintf("extbpb = %d\r\n", extbpb);
	
	if (extbpb == 0x29)
	{
		strncpy(label, secbuf + 0x2B, 11);
		label[11] = 0;

		strncpy(systyp, secbuf + 0x36, 8);
		systyp[8] = 0;
	}
	else
	{
		label[0] = 0;
		systyp[0] = 0;
	}
	cprintf("label = '%s'\r\n", label);
	cprintf("systyp = '%s'\r\n", systyp);
	
	fatstr = reserv;
	cprintf("fatstr = %d\r\n", fatstr);
	
	dirstr = reserv + fatcnt * fatsiz;
	cprintf("dirstr = %d\r\n", dirstr);
}

/****************************************************************************/

BOOL LdFat(drive)
	int drive;
{
	int i;
	
	for (i = 0; i < FATLEN; i++)
	{
		rd_lba(fat + i * SECSIZ, IOADDR, (long)fatstr + i, TRUE);
	}
}

/****************************************************************************/

LdDir16()
{
	int as, d, i, idx;
	char *buf;
	DirEnt *dir;
	char name[13];
	unsigned clus;
	int attr;
	long size;
	BOOL end;

	dircnt = 0;
	end = FALSE;
	for (as = 0; as < DIRLEN; as++)
	{
		if (end) break;
		
		rd_lba(secbuf, IOADDR, (long)dirstr + as, TRUE);
		hexdmp(secbuf, SECSIZ, 0x0000);
		/*
		if (as == 0)
			Dump(secbuf, 0, 256);
		*/

		buf = secbuf;
		for (d = 0; d < SECSIZ / 32; d++)
		{
			/*cprintf("d = %d\r\n");*/

			if (buf[0] == 0x00 || dircnt >= DIRMAX)
			{
				end = TRUE;
				break;
			}
			attr = buf[0x0B];
			if (buf[0] != 0xE5 && (attr & 0x06) == 0x00)
			{
				dir = DirList[dircnt];
				memcpy(dir->name, buf, 11);
				dir->name[11] = 0;
				if (dir->name[0] == 0x05) dir->name[0] = 0xE5;
				dir->attr = attr;
				dir->clust = getint(buf + 0x1A);
				dir->size = getlng(buf + 0x1C);
				cprintf("%s %02X %5u %6ld\r\n", dir->name, dir->attr, dir->clust, dir->size);
				dircnt++;
			}
			buf += 32;
		}
		break;
	}
	
	for (i = 0; i < dircnt; i++)
	{
		dir = DirList[i];
		cprintf("%02d %s %02X %5u %6ld\r\n", i, dir->name, dir->attr, dir->clust, dir->size);
	}
}

/****************************************************************************/

main(argc, argv)
	int argc;
	char *argv[];
{
	phyprm srcprm, dstprm;
	int p, ch;
	long lng;
	int in;
	unsigned uin;
	char *buf;

	cputs("\r\nSHOWF16 V1.0 for MC CP/M *dg* 261029-01\r\n\n");

	secbuf = calloc(SECSIZ, 1);
	if (secbuf == NULL)
	{
		cprintf("Not enough memory for sector buffer (%u)\r\n", SECSIZ);
		return FALSE;
	}

	/* allocate FAT memory */
	fat = CALLOC(SECSIZ, FATLEN);
	if (fat == NULL)
	{
		cprintf("Error: not enough memory (%d)\r\n", SECSIZ * FATLEN);
		return;
	}

	/* allocate dir list */
	DirList = CALLOC(DIRMAX, sizeof(DirEnt));
	if (DirList == NULL)
	{
		cprintf("Error: not enough memory (%d)\r\n", DIRMAX * sizeof(DirEnt));
		return;
	}

	ShowF16();
	LdDir16();

	EXIT();
}

/****************************************************************************/
