/****************************************************************************/
/*
	SHOWF12.C  *dg*  10/2026 for MI-C compiler and CP/M
	
	Show FAT12 disk

*/
/****************************************************************************/
/* SHOWF12 version history

 11.10.2026 *dg* First version

  TODO:

*/

#include "stdio.h"
#include "conio.h"
#include "phydrv.h"

#define FALSE 0
#define TRUE 1
#define WORD unsigned int
#define BOOL int

/* serial port constants */
#ASM
FALSE	EQU 0
TRUE	EQU NOT FALSE

; Monitor
MFLOP	EQU		0F021H	; Monior FLOP routine

#ENDASM

#define LF 0x0A
#define CR 0x0D
#define CTRLC 0x03

#define SECSIZ 512
char *secbuf;

#define PHYBYTS 512 /* Bytes/Sektor */
#define PHYSECS 9 /* phys. Sektoren/Track */
#define PHYTRKS 80 /* phys. Tracks */
#define PHYSIDS 2 /* phys. Sides */
#define SECSCLU 2 /* Sektoren/Cluster */
#define DATSRT 14 /* Beginn des Datenbereichs */
#define DIRSTR 7 /* Dir Startsektor 7 */
#define DIRLEN 7 /* Dir Anzahl Sektoren */
#define FATSTR 1 /* FAT Startsektor */
#define FATLEN 3 /* FAT Anzahl Sekoren */

#define CLUEND 0xFF8 /* Datei-Ende 0xFF8..0xFFF */
#define CLUDEF 0xFF7 /* defekter Sekor/Cluster  */

char *fat;

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
/* cluster -> LBA */

int clulba(clu)
	int clu;
{
	return (clu - SECSCLU) * SECSCLU + DATSRT;
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

int ReadSector(drive, track, side, sector, reg, addr)
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

int WriteSector(drive, track, side, sector, reg, addr)
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

int ReadAbs(drive, asec, buf)
	int drive, asec;
	char *buf;
{
	int sec, trk, sid;
	
	sec = abssec(asec);
	trk = abstrk(asec);
	sid = abssid(asec);
	
	/*cprintf("%d %d %d\r\n", sec, trk, sid);*/
	
	return ReadSector(drive, trk, sid, sec, 0x20, buf);
}

/****************************************************************************/

int WriteAbs(drive, asec, buf)
	int drive, asec;
	char *buf;
{
	int sec, trk, sid;

	sec = abssec(asec);
	trk = abstrk(asec);
	sid = abssid(asec);
	
	return WriteSector(drive, trk, sid, sec, 0x20, buf);
}

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

BOOL LoadFat(drive)
	int drive;
{
	int i;
	
	for (i = 0; i < FATLEN; i++)
	{
		if (!ReadAbs(drive, FATSTR + i, fat + i * PHYBYTS))
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
	int clusiz;
	int media;
	
	/*ok = ReadSector(drive, 0, 0, 1, 0x20, secbuf);*/
	ok = ReadAbs(drive, 0, secbuf);
	if (!ok)
	{
		cputs("error reading sector\r\n");
		return;
	}
	
	Dump(secbuf, 0, 128);
	
	strncpy(oem, secbuf + 3, 8);
	oem[8] = 0;
	cprintf("oem = '%s'\r\n", oem);

	secsiz = secbuf[0x0C] << 8 + secbuf[0x0B];
	cprintf("secsiz = %d\r\n", secsiz);

	clusiz = secbuf[0x0D];
	cprintf("clusiz = %d\r\n", clusiz);

	media = secbuf[0x15];
	cprintf("media = %02X\r\n", media);
}

/****************************************************************************/

ShowDir(drive)
	int drive;
{
	int as, d, i, idx;
	char *dir;
	char name[13];
	unsigned clus;
	int attr;
	long size;

	cputs("");

	idx = 0;
	for (as = 0; as < DIRLEN; as++)
	{
		ReadAbs(drive, DIRSTR + as, secbuf);
		if (as == 0)
			Dump(secbuf, 0, 256);
		
		dir = secbuf;
		for (d = 0; d < 16; d++)
		{
			attr = dir[0x0B];
			if (dir[0] != 0 && dir[0] != 0xE5 && (attr & 0x06) == 0x00)
			{
				/* name */
				strncpy(name, dir, 8);
				name[8] = '.';
				strncpy(name + 9, dir + 8, 3);
				name[12] = 0;
				cprintf("%3d %-12s ", idx + 1, name);

				clus = dir[0x1B] * 256 + dir[0x1A];
				size = (((dir[0x1F] * 256L) + dir[0x1E]) * 256L + dir[0x1D]) * 256L + dir[0x1C];
				cprintf("%02X %5u %5ld\r\n", attr, clus, size);
			}
			idx++;
			dir += 32;
		}
	}
}

/****************************************************************************/

main(argc, argv)
	int argc;
	char *argv[];
{
	int ch, p;
	int drive, port, dir, tracks;
	BOOL error;
	phyprm prm;

	cputs("SHOWF12 *dg* v1.0 281011-01\r\n\n");

	error = FALSE;
	if (argc < 2)
		error = TRUE;

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
	
	if (error)
	{
		cputs("usage: SHOWF12 <d>:\r\n");
		cputs("  <d>:  = disk drive\r\n");
		return;
	}

	/* get disk parameters */
	GetPrm(drive, &prm);
	
	/* allocate memory for sector buffer */
	secbuf = CALLOC(SECSIZ, 1);
	if (secbuf == NULL)
	{
		cprintf("Error: not enough memory (%d)\r\n", SECSIZ);
		return;
	}

	/* allocate FAT memory */
	fat = CALLOC(SECSIZ, FATLEN);
	if (fat == NULL)
	{
		cprintf("Error: not enough memory (%d)\r\n", SECSIZ);
		return;
	}

	cputs("\r\n");

	/*
	cprintf("\r\nInsert disk in drive %c and press ENTER\r\n", drive + 'A');
	ch = WaitCr();
	if (ch == CTRLC) EXIT();
	*/

	ShowFat12(drive);

	LoadFat(drive);

	ShowDir(drive);

	AllClus(22);

	/*
	cputs("\r\nInsert SYSTEM disk and press ENTER\r\n");
	WaitCr();
	*/

	EXIT();
}

/****************************************************************************/
