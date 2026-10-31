/****************************************************************************/
/*
	SHOWF12.C  *dg*  10/2026 for MI-C compiler and CP/M
	
	Show FAT12 disk

*/
/****************************************************************************/
/* SHOWF12 version history

 11.10.2026 *dg* First version
 12.10.2026 *dg* Successfully copied a file from FAT12 to CP/M
 31.10.2026 *dg* Adapted to Z80ASM

  TODO:

*/

#include "stdio.h"
#include "conio.h"
/*#include "phydrv.h"*/

#define FALSE 0
#define TRUE 1
#define WORD unsigned int
#define BOOL int

#define strncpy strncp
#define toupper touppe

/* serial port constants */
#ASM
FALSE	EQU 0
TRUE	EQU NOT FALSE
MFLOP	EQU		0F021H	; Monitor floppy routine
#ENDASM

/* set to TRUE so simulate disk access (for testing under RunCPM) */
#define SIM TRUE

#define LF 0x0A
#define CR 0x0D
#define CTRLC 0x03

#define PHYBYTS 512 /* Bytes/Sektor */
#define PHYSECS 9 /* phys. Sektoren/Track */
#define PHYTRKS 80 /* phys. Tracks */
#define PHYSIDS 2 /* phys. Sides */
#define SECCLUS 2 /* Sektoren/Cluster */
#define DATSTR 14 /* Beginn des Datenbereichs (abs. Sektor) */
#define DIRSTR 7 /* Dir Startsektor 7 (abs. Sektor) */
#define DIRLEN 7 /* Dir Anzahl Sektoren */
#define FATSTR 1 /* FAT Startsektor (abs. Sektor) */
#define FATLEN 3 /* FAT Anzahl Sekoren */
#define MAXDIR 224 /* max. Root-Eintraege (1,44 MB) */

#define CLUEND 0xFF8 /* Datei-Ende 0xFF8..0xFFF */
#define CLUDEF 0xFF7 /* defekter Sektor/Cluster  */

typedef struct
{
	char name[12]; /* not null-terminated */
	long size;
	char attr;
	unsigned clust;
} DirEnt;

DirEnt *DirList;
#define DIRMAX 128
int dircnt;

char *secbuf;

/* cluster buffer */
char *clubuf;
int clusiz;

char *fat;

#if SIM
FILE *fpsim;
#endif

/****************************************************************************/

Dump(buf, offset, len)
	char *buf;
	int offset, len;
{
	int col, p;

	col = 16;
	for (p=0; p < len; p++)
	{
		if (col >= 16)
		{
			cprintf("\r\n%04X:", offset + p);
			col = 0;
		}
		cprintf(" %02X", buf[offset + p]);
		col++;
	}
	cputs("\r\n");
}		

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
/* converts filename from dir format "FFFFFFFFXXX" to file format
   "FFFFFFF.XXX"
*/

char *fnfile(dest, src)
	char *dest, *src;
{
	int i;
	char *s;
	
	s = src;
	for (i = 0; i < 8; i++)
	{
		if (*s == ' ') break;
		*dest++ = *s++;
	}
	*dest++ = '.';
	s = src + 8;
	for (i = 0; i < 3; i++)
	{
		if (*s == ' ') break;
		*dest++ = *s++;
	}
	if (*(dest-1) == '.')
		dest--;
	
	*dest++ = 0;
	return dest;
}

/****************************************************************************/
/* converts filename from file format "FFFFFFFF.XXX" to dir format
   "FFFFFFFXXX"
*/

char *fndir(dest, src)
	char *dest, *src;
{
	int i;
	char *d;

	for (i = 0; i < 11; i++)
		dest[i] = ' ';
	dest[11] = 0;

	d = dest;
	for (i = 0; i < 12; i++)
	{
		if (*src == 0) break;
		if (*src == '.')
		{
			d = dest + 8;
			src++;
			continue;
		}
		*d++ = *src;
		src++;
	}
	return dest;
}

/****************************************************************************/
/* cluster -> LBA */

int clulba(clu)
	int clu;
{
	return (clu - SECCLUS) * SECCLUS + DATSTR;
}

/****************************************************************************/
/* absolute sector -> phys. sector (0..n-1) */

int abssec(asec)
	int asec;
{
	return (asec % PHYSECS) + 1;
}

/****************************************************************************/
/* absolute sector -> side (0..1) */

int abssid(asec)
	int asec;
{
	int t;
	
	t = asec / PHYSECS;
	return t % PHYSIDS;
}

/****************************************************************************/
/* absolute sector -> side (0..1) */

int abstrk(asec)
	int asec;
{
	int t;
	
	t = asec / PHYSECS;
	return t / PHYSIDS;
}

/****************************************************************************/
/* Monitor: read pyhsical sector */
/*
; HL = Buffer address
; E = Physical sector (1..n)
; D = Track (0..n-1)
; C = Drive-code + Density + Side (REG4-Latch)
; return 0 = no error
*/

BOOL Flop(cmd, drvcod, track, sector, addr)
	int cmd, drvcod, track, sector;
	unsigned addr;
{
#ASM
	LD IX,2		; RET
	ADD IX,SP
	LD B,(IX+0)	; cmd (1= read, 2=write)
	LD C,(IX+2)	; drive code
	LD D,(IX+4)	; track
	LD E,(IX+6)	; sector
	LD L,(IX+8)	; addr (high)
	LD H,(IX+9)	; addr (low)
	CALL MFLOP	; Monitor FLOP routine
	LD L,A		; A=0: no error
	LD H,0
#ENDASM
}

/****************************************************************************/
/* drive 0..n-1 */
/* track 0..n-1 */
/* side 0..n.1 */
/* sector 0..n-1 */
/* FLO register */
/* addr = buffer address */

int RdSect(drive, track, side, sector, reg, addr)
	int drive, track, side, sector, reg;
	unsigned addr;
{
	int drvcod;
	
	/*xprintf("%d %d %d %d %02X %04X\r\n", drive, track, side, sector, reg, addr);*/
	
	/*int drvcod = (1 << drive) | 0x20;*/
	drvcod = (1 << drive) | reg;
	if (side == 1) drvcod |= 0x80;
	 /* read sector by Monitor */
	return Flop(1, drvcod, track, sector, addr) == 0;
}

/****************************************************************************/
/* drive 0..n-1 */
/* track 0..n-1 */
/* side 0..n.1 */
/* sector 0..n-1 */
/* FLO register */
/* addr = buffer address */

int WrSect(drive, track, side, sector, reg, addr)
	int drive, track, side, sector, reg;
	unsigned addr;
{
	int drvcod;
	/*cprintf("\r\nWR %d %d %d %d %u\t\n", drive, track, side, sector, addr);*/
	
	/*int drvcod = (1 << drive) | 0x20;*/
	drvcod = (1 << drive) | reg;
	if (side == 1) drvcod |= 0x80;
	 /* write sector by Monitor */
	return Flop(2, drvcod, track, sector, addr) == 0;
}

/****************************************************************************/
/* asec = absolute sector (0..n-1) */

int RdAbs(drive, asec, buf)
	int drive;
	unsigned asec;
	char *buf;
{
	int sec, trk, sid;

#if SIM
	/* read from image instead from real disk */
	long offset;
	offset = asec * PHYBYTS;
	fseek(fpsim, offset, 0);
	return fread(buf, PHYBYTS, 1, fpsim) == 1;
#ELSE
	sec = abssec(asec);
	trk = abstrk(asec);
	sid = abssid(asec);

	/*cprintf("%d %d %d\r\n", sec, trk, sid);*/

	return RdSect(drive, trk, sid, sec, 0x20, buf);
#endif
}

/****************************************************************************/
/* read cluster */

int RdClus(drive, clus, buf)
	int drive;
	unsigned clus;
	char *buf;
{
	int i;
	int	asec;

	asec = DATSTR + (clus - 2) * SECCLUS;
	
	for (i = 0; i < SECCLUS; i++)
	{
		if (!RdAbs(drive, asec + i, buf + i * PHYBYTS))
			return FALSE;
	}
	return TRUE;
}

/****************************************************************************/
/* asec = absolute sector (0..n-1) */

int WrAbs(drive, asec, buf)
	int drive;
	unsigned asec;
	char *buf;
{
	int sec, trk, sid;

	sec = abssec(asec);
	trk = abstrk(asec);
	sid = abssid(asec);
	
	return WrSect(drive, trk, sid, sec, 0x20, buf);
}

/****************************************************************************/

BOOL LdFat(drive)
	int drive;
{
	int i;
	
	for (i = 0; i < FATLEN; i++)
	{
		if (!RdAbs(drive, FATSTR + i, fat + i * PHYBYTS))
			return FALSE;
	}
}

/****************************************************************************/

unsigned NxtClus(clus)
	unsigned clus;
{
	unsigned ofs;

	ofs = clus + clus / 2; /* x1,5 */
	
	if ((clus % 2) == 0)
	{
		/* even */
		return (fat[ofs + 1] * 256 + fat[ofs]) & 0xFFF; /* 12 bit */
	}
	else
	{
		return (fat[ofs + 1] * 256 + fat[ofs]) >> 4; /* 12 bit */
		/* odd */
	}
}

/****************************************************************************/

AllClus(clus)
	unsigned clus;
{
	int cnt;

	cnt = 1;
	while(TRUE)
	{
		cprintf("%d %d\r\n", cnt + 1, clus);
		clus = NxtClus(clus);
		if (clus >= 0xFF8) break;
		if (++cnt > 60) break;
	}
}

/****************************************************************************/

ShowFat12(drive)
	int drive;
{
	BOOL ok;
	char oem[9];
	int secsiz;
	int clucnt;
	int media;
	
	/*ok = RdSect(drive, 0, 0, 1, 0x20, secbuf);*/
	ok = RdAbs(drive, 0, secbuf);
	if (!ok)
	{
		cputs("error reading sector\r\n");
		return;
	}
	
	/*Dump(secbuf, 0, 128);*/
	
	strncpy(oem, secbuf + 3, 8);
	oem[8] = 0;
	cprintf("oem = '%s'\r\n", oem);

	secsiz = secbuf[0x0C] << 8 + secbuf[0x0B];
	cprintf("secsiz = %d\r\n", secsiz);

	clucnt = secbuf[0x0D];
	cprintf("clucnt = %d\r\n", clucnt);
	clusiz = clucnt * PHYBYTS;

	media = secbuf[0x15];
	cprintf("media = %02X\r\n", media);
}

/****************************************************************************/
/*	Load a file from source disk and write it to fname

	drive: source drive
	fname: filename (with drive)
	fsize: filesize (in bytes)
	clus: start cluster from directory
*/
BOOL LdFile(drive, fname, fsize, clus)
	int drive;
	char *fname;
	unsigned fsize, clus;
{
	int cnt;
	int asec;
	unsigned size, l;
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

		if (!RdClus(drive, clus, clubuf))
		{
			cprintf("error reading cluster %d\r\n", clus);
			error = TRUE;
			break;
		}

		cnt++;
		wrsize = PHYBYTS * SECCLUS;
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

LdDir(drive)
	int drive;
{
	int as, d, i, idx;
	char *buf;
	DirEnt *dir;
	char name[13];
	unsigned clus;
	int attr;
	long size;

	dircnt = 0;
	for (as = 0; as < DIRLEN; as++)
	{
		RdAbs(drive, DIRSTR + as, secbuf);
		/*
		if (as == 0)
			Dump(secbuf, 0, 256);
		*/
		
		buf = secbuf;
		for (d = 0; d < 16; d++)
		{
			attr = buf[0x0B];
			if (buf[0] != 0 && buf[0] != 0xE5 && (attr & 0x06) == 0x00)
			{
				dir = DirList[dircnt];
				memcpy(dir->name, buf, 11);
				dir->name[11] = 0;
				dir->attr = attr;
				dir->clust = buf[0x1B] * 256 + buf[0x1A];
				dir->size = (((buf[0x1F] * 256L) + buf[0x1E]) * 256L + buf[0x1D]) * 256L + buf[0x1C];
				cprintf("%s %02X %5u %6ld\r\n", dir->name, dir->attr, dir->clust, dir->size);
				dircnt++;
			}
			buf += 32;
		}
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
	int ch, p, i;
	int drive, port, dir, tracks;
	BOOL error;
	/*phyprm prm;*/
	char dest1[13], dest2[13];

	cputs("SHOWF12 *dg* v1.0 281012-01\r\n\n");

	/*
	fnfile(dest1, "ABC     EXT");
	cprintf("'%s'\r\n", dest1);
	fndir(dest2, dest1);
	cprintf("'%s'\r\n", dest2);

	fnfile(dest1, "ABCDEFGHEXT");
	cprintf("'%s'\r\n", dest1);
	fndir(dest2, dest1);
	cprintf("'%s'\r\n", dest2);

	fnfile(dest1, "ABCDEFGHEX ");
	cprintf("'%s'\r\n", dest1);
	fndir(dest2, dest1);
	cprintf("'%s'\r\n", dest2);

	fnfile(dest1, "ABCDEFGH   ");
	cprintf("'%s'\r\n", dest1);
	fndir(dest2, dest1);
	cprintf("'%s'\r\n", dest2);
	*/

	drive = 1;
	error = FALSE;
	/*
	if (argc < 2)
		error = TRUE;
	*/

	if (!error)
	{
		for (p = 1; p < argc; p++)
		{
			if (strlen(argv[p]) == 2 && argv[p][1] == ':')
			{
				drive = toupper(argv[p][0]) - 'A';
				if (drive < 0 || drive > 15) error = TRUE;
				continue;
			}
		}
	}
	
	if (drive == -1 || error)
	{
		cputs("usage: SHOWF12 <d>:\r\n");
		cputs("  <d>:  = disk drive\r\n");
		return;
	}

	/* get disk parameters */
	/*
	GetPrm(drive, &prm);
	*/
	
	/* allocate memory for sector buffer */
	secbuf = CALLOC(PHYBYTS, 1);
	if (secbuf == NULL)
	{
		cprintf("Error: not enough memory (%d)\r\n", PHYBYTS);
		return;
	}

	/* allocate FAT memory */
	fat = CALLOC(PHYBYTS, FATLEN);
	if (fat == NULL)
	{
		cprintf("Error: not enough memory (%d)\r\n", PHYBYTS * FATLEN);
		return;
	}

	/* allocate dir list */
	DirList = CALLOC(DIRMAX, sizeof(DirEnt));
	if (DirList == NULL)
	{
		cprintf("Error: not enough memory (%d)\r\n", DIRMAX * sizeof(DirEnt));
		return;
	}

	for (i = 0; i < 3; i++)
	{
		cprintf("%d %04X\r\n", i, DirList[i]);
	}

	cputs("\r\n");

	/*
	cprintf("\r\nInsert disk in drive %c and press ENTER\r\n", drive + 'A');
	ch = WaitCr();
	if (ch == CTRLC) EXIT();
	*/

#if SIM
	/* open a 720k DOS image to simulate a real DOS disk */
	fpsim = fopen("msdos.img", "r");
	if (fpsim == NULL)
	{
		cprintf("error opening MSDOS.IMG\r\n");
		return;
	}
#endif

	ShowFat12(drive);

	LdFat(drive);

	/* allocate cluster buffer */
	clubuf = CALLOC(clusiz, 1);
	if (fat == NULL)
	{
		cprintf("Error: not enough memory (%d)\r\n", clusiz);
		return;
	}

	/*
	AllClus(22);
	*/

	LdDir(drive);

	/*
	LdFile(drive, "F:MSDOS.COM", 25088, 22);
	*/

	/*
	cputs("\r\nInsert SYSTEM disk and press ENTER\r\n");
	WaitCr();
	*/

	EXIT();
}

/****************************************************************************/
