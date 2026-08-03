/****************************************************************************/
/*
	DISKTEST.C  *dg*  08/2026 for MI-C compiler and MC CP/M clone
	
	Write individual pattern to each logical sector and read it back.
	Blocking/Deblocking test
*/
/****************************************************************************/
/* DISKTEST version history

 03.08.2026 *dg* First version
 
 */

#include "stdio.h"

#define FALSE 0
#define TRUE 1
#define WORD unsigned int
#define BOOL int

#define CTRLC 3
#define CR 13

/* format definiion */
#define TRACKS 80
#define SIDES 2
/*#define SECTORS 54*/
#define SECTOR_SIZE 128

#define DRV_CNT 8 /* number of drives */

#define DEBDRV 1
#define DEBSEC 56

char outbuf[80];

char *buffer;

#ASM
BIOS:	DW	0
#ENDASM

/****************************************************************************/

unsigned int BiosAddr()
{
#ASM
	LD HL,(0001H)	; BIOS address
	DEC HL
	DEC HL
	DEC HL
	LD (BIOS),HL
#ENDASM
}

/****************************************************************************/

IsRunCpm()
{
	return BiosAddr() == 0XEE00;
}

/****************************************************************************/

int InPort(addr)
	int addr;
{
#ASM
	POP DE
	POP BC		; addr in C
	PUSH BC
	PUSH DE
	IN L,(C)	; in
	LD H, 0
#ENDASM	
}

/****************************************************************************/

OutPort(addr, byte)
	int addr, byte;
{
#ASM
	POP IX
	POP BC		; addr in C
	POP HL
	PUSH HL		; byte in L
	PUSH BC
	PUSH IX
	OUT (C),L	; out
	LD H,0
#ENDASM	
}

/****************************************************************************/
/* BIOS select drive for read/write */
/* returns pointer to drive parameter table */

char *SelDrv(drive)
	int drive;
{
#ASM
	POP BC
	POP HL
	PUSH HL
	PUSH BC
	LD C,L
	LD HL,(BIOS)
	LD DE,001BH		; function SELDSK
	ADD HL,DE
	LD DE,seldrv1
	PUSH DE
	JP (HL)
seldrv1:
#ENDASM	
}

/****************************************************************************/
/* BIOS set track for read/write */

SetTrk(track)
	int track;
{
#ASM
	POP BC
	POP HL
	PUSH HL
	PUSH BC
	LD C,L
	LD HL,(BIOS)
	LD DE,001EH		; function SETTRK
	ADD HL,DE
	LD DE,settrk1
	PUSH DE
	JP (HL)
settrk1:
#ENDASM	
}

/****************************************************************************/
/* BIOS set sector for read/write */

SetSec(sector)
	int sector;
{
#ASM
	POP BC
	POP HL
	PUSH HL
	PUSH BC
	LD C,L
	LD HL,(BIOS)
	LD DE,0021H		; function SETSEC
	ADD HL,DE
	LD DE,setsec1
	PUSH DE
	JP (HL)
setsec1:
#ENDASM	
}

/****************************************************************************/
/* BIOS set buffer address for read/write */

SetDma(dma)
	char *dma;
{
#ASM
	POP BC			; DMA
	POP HL
	PUSH HL
	PUSH BC
	LD B,H
	LD C,L
	LD HL,(BIOS)
	LD DE,0024H		; function SETDMA
	ADD HL,DE
	LD DE,setdma1
	PUSH DE
	JP (HL)
setdma1:
#ENDASM	
}

/****************************************************************************/
/* BIOS read sector from disk */
/* call BiosAddr to initialise BIOS start address !!! */

BiosRead()
{
#ASM
	LD HL,(BIOS)
	LD DE,0027H		; function DISK READ
	ADD HL,DE
	LD DE,read1
	PUSH DE
	JP (HL)
read1:
	LD L,A
	LD H,0
#ENDASM	
}

/****************************************************************************/
/* BIOS write sector to disk */

BiosWrite()
{
#ASM
	LD C,0			; flag for directory sector from BDOS
	LD HL,(BIOS)
	LD DE,002AH		; function DISK WRITE
	ADD HL,DE
	LD DE,write1
	PUSH DE
	JP (HL)
write1:
	LD L,A
	LD H,0
#ENDASM	
}

/****************************************************************************/
/* BDOS init disk system */

InitDisks()
{
	BDOS(0, 0x0D);
}

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
		BD_CoOut(*s++);
}		

/****************************************************************************/

int WaitCr()
{
	int ch;
	
	while(TRUE)
	{
		ch = GetChr();
		if (ch == CTRLC || ch == CR) return ch; /* Ctrl-C or ENTER */
	}
}

/****************************************************************************/

int ReadSector(drive, track, side, sector, addr)
	int drive, track, side, sector;
	unsigned addr;
{
	int result;
	
	SelDrv(drive);
	SetTrk(track * 2 + side);
	SetSec(sector);
	SetDma(addr);
	return BiosRead(); /* read sector by BIOS */
}

/****************************************************************************/
/* drive 0..n-1 */
/* track 0..n-1 */
/* side 0..n.1 */
/* sector 0..n-1 */
/* addr = buffer address */

int WriteSector(drive, track, side, sector, addr)
	int drive, track, side, sector;
	unsigned addr;
{
	int result;
	/*printf("\r\nWR %d %d %d %d %u\t\n", drive, track, side, sector, addr);*/
	
	SelDrv(drive);
	SetTrk(track * 2 + side);
	SetSec(sector);
	SetDma(addr);
	return BiosWrite();  /* write sector by BIOS */
}

/****************************************************************************/

BOOL WriteDisk(drive, seccnt, buffer)
	int drive, seccnt;
	char *buffer;
{
	int track, side, sector, b;
	int ch, stat, result;

	/* for (track = 0; track < TRACKS; track++) */
	for (track = 0; track < 2; track++)
	{
		ch = GetChr();
		if (ch == CTRLC) 
		{
			printf("\r\nCtrl-C detected. Aborted!\r\n");
			return FALSE;
		}

		/*printf("Write track %d %04X\r\n", track, buffer);*/

		for (side = 0; side < SIDES; side++)
		{
			for (sector = 0; sector < seccnt; sector++)
			{
				printf("Write track=%d side=%d sector=%d %04X\r\n", track, side, sector, buffer);
				
				buffer[0] = track;
				buffer[1] = side;
				buffer[2] = sector;
				result = WriteSector(drive, track, side, sector, buffer);

				if (result != 0)
				{
					stat = InPort(0xC0);
				
					printf("\r\nWrite error drive %c track %d side=%d sector=%d stat=%02X\r\n",
						drive + 'A', track, side, sector, stat);
					if (stat & 0x40)
						printf("\r\nWrite protection");
					return FALSE;
				}
			}
		}
	} /* tracks */
	return TRUE;
}

/****************************************************************************/

BOOL ReadDisk(drive, seccnt, buffer)
	int drive, seccnt;
	char *buffer;
{
	int track, side, sector, b;
	int ch, stat, result;
	BOOL error;

	/* for (track = 0; track < TRACKS; track++) */
	for (track = 0; track < 2; track++)
	{
		ch = GetChr();
		if (ch == CTRLC) 
		{
			printf("\r\nCtrl-C detected. Aborted!\r\n");
			return FALSE;
		}

		/*printf("Read track %d %04X\r\n", track, buffer);*/

		for (side = 0; side < SIDES; side++)
		{
			for (sector = 0; sector < seccnt; sector++)
			{
				printf("Read track=%d side=%d sector=%d %04X\r\n", track, side, sector, buffer);
				
				result = ReadSector(drive, track, side, sector, buffer);
				if (result != 0)
				{
					stat = InPort(0xC0);
				
					printf("\r\nRead error drive %c track %d side=%d sector=%d stat=%02X\r\n",
						drive + 'A', track, side, sector, stat);
					return FALSE;
				}

				error = FALSE;
				if (buffer[0] != track)
				{
					error = TRUE;
					printf("\r\ntrack error track=%d read=%d\r\n", track, buffer[0]);
				}
				if (buffer[1] != side)
				{
					error = TRUE;
					printf("\r\nside error side=%d read=%d\r\n", side, buffer[1]);
				}
				if (buffer[2] != sector)
				{
					error = TRUE;
					printf("\r\nsector error sector=%d read=%d\r\n", sector, buffer[2]);
				}
				if (error)
				{
					ch = WaitCr();
					if (ch==CTRLC) return FALSE;
				}
			}
		}
	} /* tracks */
	return TRUE;
}

/****************************************************************************/

main(argc, argv)
	int argc;
	char *argv[];
{
	int drive, seccnt, b, p, ch;
	BOOL error, success;

	BiosAddr();
	
	PutStr("\r\nDISKTEST V1.0 *dg* 08/2026\r\n\n");

	seccnt = 40;

#ifndef DEBDRV
	error = FALSE;
	
	if (argc < 2)
		error = TRUE;
	else
	{
		drive = argv[1][0] - 'A';
		error = strlen(argv[1]) != 2 ||
				 drive < 0 || drive >= DRV_CNT ||
				 argv[1][1] != ':';
	}
	
	if (!error)
	{
		for (p = 2; p < argc; p++)
		{
			if (argv[p][0] != '-' && argv[p][0] != '/') continue;
			if (toupper(argv[p][1]) == 'S' && strlen(argv[p]) >= 2)
			{
				seccnt = atoi(argv[p] + 2);
				if (seccnt< 1 || seccnt > 255)
					error = TRUE;
			}
		}
	}
	
	if (error)
	{
		PutStr("Usage: disktest d: S<s>\r\n");
		PutStr("  d = Drive A..D\r\n");
		PutStr("  s = Sector count \r\n");
		return;
	}
#else	
	drive = DEBDRV;
	seccnt = 56;
#endif

	buffer = CALLOC(SECTOR_SIZE, 1);
	if (buffer == NULL)
	{
		PutStr("Error: not enough memory\r\n");
		return FALSE;
	}

	sprintf(outbuf, "sectors = %d\r\n", seccnt);
	PutStr(outbuf);

	sprintf(outbuf, "\r\nInsert disk in drive %c: an press ENTER\r\n\r\n", drive + 'A');
	PutStr(outbuf);
	ch = WaitCr();
	if (ch == CTRLC) return;

	InitDisks();

	for (b = 0; b < SECTOR_SIZE; b++)
		buffer[b] = 0x55;

	WriteDisk(drive, seccnt, buffer);
	
	ReadDisk(drive, seccnt, buffer);

	PutStr("\r\n\nInsert SYSTEM-Disk and press ENTER\r\n");
	WaitCr();

	InitDisks();
}

/****************************************************************************/
