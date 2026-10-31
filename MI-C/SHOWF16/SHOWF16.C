/****************************************************************************/
/*
	SHOWF16.C  *dg*  10/2026 for MI-C compiler and MC CP/M clone
	
	Show FAT16 formated compact flash (without MBR)
*/
/****************************************************************************/
/* SHOWF16 version history

 29.10.2026 *dg* First version (from COPYCF and SHOWF12)
 31.10.2026 *dg* Adapted to Z80ASM
 
 */

#include "stdio.h"
#include "phydrv.h"

#define FALSE 0
#define TRUE 1
#define WORD unsigned int
#define BOOL int

#define strncpy strncp

#define CTRLC 3
#define CR 13

#define IOADDR 0x18

#define VOLCNT 4 /* number of volumes */

/*#define SECSIZ 512*/
char *secbuf;
int secsiz; /* bytes per sector (512) */
char *clubuf;
int clusiz; /* bytes per cluster (1024) */

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
int fatsiz; /* sectors per FAT */
int clucnt;	/* sectors per cluster */
int datstr; /* erster Datencluster */

int fatstr; /* FAT start sector */
char *fatbuf; /* FAT cache sector */
int lstfat = 0; /* last read FAT sector */

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

LdBoot16()
{
	BOOL ok;
	char oem[8+1]; /* oem name */
	char label[11+1];
	char systyp[8+1]; /* file system type ('FAT16   ') */
	int reserv; /* reserved sectors before first FAT (8) */
	int fatcnt; /* number pf FATs (2) */
	int maxdir; /* max. number of root dir entries */
	int media; /* media descriptor (F8) */
	int seccnt;
	int extbpb;

	rd_lba(secbuf, IOADDR, 0L, TRUE);
	hexdmp(secbuf, 512, 0x0000);
	
	strncpy(oem, secbuf + 3, 8); /* oem name */
	oem[8] = 0;
	cprintf("oem = '%s'\r\n", oem);

	secsiz = getint(secbuf + 0x0B); /* bytes per sector */
	cprintf("secsiz = %d\r\n", secsiz);

	clucnt = secbuf[0x0D]; /* sectors per cluster */
	clusiz = clucnt * secsiz; /* bytes per cluster */
	cprintf("clucnt = %d\r\n", clucnt);
	cprintf("clusiz = %d\r\n", clusiz);

	reserv = getint(secbuf + 0x0E); /* reserved sectors before first FAT (8) */
	cprintf("reserv = %d\r\n", reserv);
	
	fatcnt = secbuf[0x10]; /* number pf FATs (2) */
	cprintf("fatcnt = %d\r\n", fatcnt);
	
	maxdir = getint(secbuf + 0x11); /* max. number of root dir entries */
	cprintf("maxdir = %d\r\n", maxdir);

	seccnt = getint(secbuf + 0x13); /* bytes per sector */
	cprintf("seccnt = %d\r\n", seccnt);

	media = secbuf[0x15]; /* media descriptor */
	cprintf("media = %02X\r\n", media);

	fatsiz = getint(secbuf + 0x16); /* sectors per FAT */
	cprintf("fatsiz = %d\r\n", fatsiz);

	extbpb = secbuf[0x26];
	cprintf("extbpb = %d\r\n", extbpb);
	
	if (extbpb == 0x29)
	{
		strncpy(label, secbuf + 0x2B, 11);
		label[11] = 0;

		strncpy(systyp, secbuf + 0x36, 8); /* file system type ('FAT16   ') */
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
	
	datstr = reserv + fatcnt * fatsiz + maxdir * 32 / secsiz;
	cprintf("datstr = %d\r\n", datstr);
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
		hexdmp(secbuf, secsiz, 0x0000);
		/*
		if (as == 0)
			Dump(secbuf, 0, 256);
		*/

		buf = secbuf;
		for (d = 0; d < secsiz / 32; d++)
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
/* read cluster */

int RdClus(clus, buf)
	unsigned clus;
	char *buf;
{
	int i;
	int	asec;

	asec = datstr + (clus - 2) * clucnt;
	
	for (i = 0; i < clucnt; i++)
	{
		rd_lba(buf + i * secsiz, IOADDR, (long)asec + i, TRUE);
	}
	return TRUE;
}

/****************************************************************************/

unsigned NxtClus(clus)
	unsigned clus;
{
	int sec;
	
	sec = fatstr + clus * 2 / secsiz;
	cprintf("sec=%d / %d\r\n", sec, clus * 2);
	
	if (sec != lstfat)
	{
		/* FAT sector not in cache */
		rd_lba(fatbuf, IOADDR, (long)sec, TRUE);
		lstfat = sec;
	}

	return getint(fatbuf + clus * 2);
}

/****************************************************************************/
/*	load a file from Compact-Flash and write it to fname

	fname: filename (with drive)
	fsize: filesize (in bytes)
	clus: start cluster from directory
*/
BOOL LdFile(fname, fsize, clus)
	char *fname;
	unsigned fsize, clus;
{
	int cnt, asec;
	unsigned size;
	int wrsize;
	BOOL error;
	FILE *fp;

	fp = fopen(fname, "w");
	if (fp == NULL) return FALSE;

	cnt = 0; /* cluster count */
	size = 0;
	error = FALSE;
	while(TRUE)
	{
		cprintf("cnt=%d clus=%d\r\n", cnt + 1, clus);

		if (!RdClus(clus, clubuf))
		{
			cprintf("error reading cluster %d\r\n", clus);
			error = TRUE;
			break;
		}

		cnt++;
		wrsize = secsiz * clucnt;
		if (size + wrsize > fsize)
		{
			wrsize = fsize - size;
		}
		cprintf("write cnt=%d clus=%d %u\r\n", cnt, clus, wrsize);
		
		if (fwrite(clubuf, wrsize, 1, fp) != 1)
		{
			cprintf("error writing cluster %d\r\n", cnt);
			error = TRUE;
			break;
		}
		size += wrsize;
		clus = NxtClus(clus);
		cprintf("nxt clus=%d\r\n", clus);
		
		if (clus >= 0xFF8) break;
	}
	cprintf("size=%u\r\n", size);
	fclose(fp);
}

/****************************************************************************/
/* test read only the cluster chain, not the actual file */

AllClus(clus)
	unsigned clus;
{
	int cnt;
	int size;

	cnt = 0;
	size = 0;
	while(TRUE)
	{
		cnt++;
		size += clusiz;
		cprintf("allclus: #%d %d %d\r\n", cnt, clus, size);
		clus = NxtClus(clus);
		if (clus >= 0xFF8) break;
		if (cnt > 60) break;
	}
}

/****************************************************************************/

main(argc, argv)
	int argc;
	char *argv[];
{
	cputs("\r\nSHOWF16 V1.0 for MC CP/M *dg* 261031-01\r\n\n");

	secbuf = calloc(512, 1);
	if (secbuf == NULL)
	{
		cprintf("Not enough memory for sector buffer (%u)\r\n", 512);
		return FALSE;
	}

	LdBoot16();

	/* allocate cluster buffer */
	clubuf = CALLOC(clusiz, 1);
	if (clubuf == NULL)
	{
		cprintf("Error: not enough memory for cluster buffer (%u)\r\n", clusiz);
		return;
	}

	/* allocate cluster buffer */
	fatbuf = CALLOC(secsiz, 1);
	if (fatbuf == NULL)
	{
		cprintf("Error: not enough memory for cluster buffer (%u)\r\n", secsiz);
		return;
	}

	/* allocate dir list */
	DirList = CALLOC(DIRMAX, sizeof(DirEnt));
	if (DirList == NULL)
	{
		cprintf("Error: not enough memory for directory (%d)\r\n", DIRMAX * sizeof(DirEnt));
		return;
	}
	
	LdDir16();

	/*AllClus(17);*/
	
	/* load MABSIC from CF */
	LdFile("mbasic.com", 24320, 17);

	EXIT();
}

/****************************************************************************/
