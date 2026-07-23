/****************************************************************************/
/*
	DISKCOPY.C  *dg*  07/2026 for MI-C compiler and MC CP/M clone
	
	Copy a disk from one drive to another drive or on one drive.
	Only for format: NDR - DD/DS/80 track/800 KB.
*/
/****************************************************************************/
/* DISKCOPY version history

 23.07.2026 *dg* First version, copy on one drive not implemented yet
 
 */

#include "stdio.h"

#define FALSE 0
#define TRUE 1
#define WORD unsigned int
#define BOOL int

#define CTRLC 3
#define CR 13

#define USEMON TRUE
/*
#define DEBSRC 0
#define DEBDEST 1
*/

/* format definiion */
#define TRACKS 80

#if USEMON == FALSE

#define BYTES_PER_SECTOR 128
#define SECTORS 40

#else

#define BYTES_PER_SECTOR 1024
#define SECTORS 5

#endif

#define SIDES 2
#define SIDE 0 /* side if SS */

#define DRV_CNT 8 /* number of drives */

char *buffer;
char *vfy_buffer;

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
	;LD HL,(0001H)	; BIOS address
	;DEC HL
	;DEC HL
	;DEC HL
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

/*
Test(a, b, c, d, e)
	unsigned a, b, c, d, e;
{
#ASM
	LD IX,2
	ADD IX,SP
	LD B,(IX+0)	; a low
	LD C,(IX+1)	; a high
	LD D,(IX+2)	; b low
	LD E,(IX+3)	; b high
	LD H,(IX+4)	; c low
	LD L,(IX+5)	; c high
#ENDASM
}
*/

/****************************************************************************/
/* Monitor: read pyhsical sector */
/*
; HL = Buffer address
; E = Physical sector (1..n)
; D = Track (0..n-1)
; C = Drive-code + Density + Side (REG4-Latch)
; return 0 = no error
*/

#if USEMON == TRUE

BOOL Flop(cmd, drive, track, sector, addr)
	int cmd,drive, track, sector;
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

	;POP IX	; RET
	;POP HL	; cmd in L (1= read, 2=write)
	;LD B,L
	;POP HL	; drive code in L
	;LD C,L
	;POP HL	; track in L
	;LD D,L
	;POP HL	; sector in L
	;LD E,L
	;POP HL	; addr
	;PUSH HL
	;PUSH HL
	;PUSH HL
	;PUSH HL
	;PUSH HL
	;PUSH IX	; RET

	CALL 0F021H		; Monitor FLOP routine
	LD L,A			; A=0: no error
	LD H,0
#ENDASM
}

#endif

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
	
#if USEMON == FALSE
	SelDrv(drive);
	SetTrk(track * 2 + side);
	SetSec(sector);
	SetDma(addr);
	return BiosRead(); /* read sector by BIOS */
#else
	int drvcod = (1 << drive) | 0x20;
	if (side == 1) drvcod |= 0x80;
	 /* read sector by Monitor */
	return Flop(1, drvcod, track, sector + 1, addr);
#endif
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
	
#if USEMON == FALSE
	SelDrv(drive);
	SetTrk(track * 2 + side);
	SetSec(sector);
	SetDma(addr);
	return BiosWrite();  /* write sector by BIOS */
#else
	int drvcod = (1 << drive) | 0x20;
	if (side == 1) drvcod |= 0x80;
	 /* write sector by Monitor */
	return Flop(2, drvcod, track, sector + 1, addr); /* write sector */
#endif
}

/****************************************************************************/
/* read one track */

BOOL ReadTrack(drive, track, buffer)
	int drive, track;
	unsigned buffer;
{
	unsigned offset;
	int side, sector, drvcod;
	int result;

#if USEMON == FALSE
	SelDrv(drive);
#endif
	for (side = SIDE; side < SIDES; side++)
#if USEMON == FALSE
		SetTrk(track * 2 + side);
#endif
		for (sector = 0; sector < SECTORS; sector++)
		{
#if FALSE			
			offset = sector * (BYTES_PER_SECTOR * SIDES) + side * BYTES_PER_SECTOR;
			result = ReadSector(drive, track, side, sector, buffer + offset);
#endif			
			
#if USEMON == FALSE
			SetSec(sector);
			offset = sector * (BYTES_PER_SECTOR * SIDES) + side * BYTES_PER_SECTOR;
			SetDma(buffer + offset);
			result = BiosRead(); /* read sector by BIOS */
#else
			drvcod = (1 << drive) | 0x20;
			if (side == 1) drvcod |= 0x80;
			offset = sector * (BYTES_PER_SECTOR * SIDES) + side * BYTES_PER_SECTOR;
			 /* read sector by Monitor */
			result = Flop(1, drvcod, track, sector + 1, buffer + offset);
#endif

			if (result != 0) return FALSE;
		}
	return TRUE;
}

/****************************************************************************/
/* write one track */

BOOL WriteTrack(drive, track, buffer)
	int drive, track;
	unsigned buffer;
{
	unsigned offset;
	int side, sector, drvcod;
	int result;

#if USEMON == FALSE
	SelDrv(drive);
#endif
	for (side = SIDE; side < SIDES; side++)
#if USEMON == FALSE
		SetTrk(track * 2 + side);
#endif
		for (sector = 0; sector < SECTORS; sector++)
		{
#if FALSE			
			offset = sector * (BYTES_PER_SECTOR * SIDES) + side * BYTES_PER_SECTOR;
			result = WriteSector(drive, track, side, sector, buffer + offset);
#endif
			
#if USEMON == FALSE
			SetSec(sector);
			offset = sector * (BYTES_PER_SECTOR * SIDES) + side * BYTES_PER_SECTOR;
			SetDma(buffer + offset);
			result = BiosWrite();  /* Write sector by BIOS */
#else
			drvcod = (1 << drive) | 0x20;
			if (side == 1) drvcod |= 0x80;
			offset = sector * (BYTES_PER_SECTOR * SIDES) + side * BYTES_PER_SECTOR;
			 /* write sector by Monitor */
			result = Flop(2, drvcod, track, sector + 1, buffer + offset); /* write sector */
#endif
			if (result != 0) return FALSE;
		}
	return TRUE;
}

/****************************************************************************/
/* copy using 1 drive */

BOOL Copy1Drive(drive)
	int drive;
{
	unsigned bufsize;
	unsigned tracksize, s;
	int tracks;

	PutStr("Copy on one drive not implemented yet");
	return FALSE;


	tracksize = SECTORS * BYTES_PER_SECTORS * SIDES;
	/*printf("tracksize=%d\r\n", tracksize);*/
	
	for (tracks = 5; tracks > 0; tracks--)
	{
		bufsize = tracks * tracksize;
		buffer = CALLOC(SECTORS * BYTES_PER_SECTORS * SIDES, 1);
		if (buffer != NULL) break;
	}
	
	if (tracks == 0)
	{
		PutStr("Error: not enough memory\r\n");
		return FALSE;
	}
	
	printf("tracks = %d, changes = %d\r\n", tracks, TRACKS / tracks + 1);
	
}

/****************************************************************************/

BOOL Copy2Drives(src, dest)
	int src, dest;
{
	int track, ch, stat;
	BOOL success;

	for (track = 0; track < TRACKS; track++)
	{
		ch = GetChr();
		if (ch == CTRLC) 
		{
			printf("\r\nCtrl-C detected. Aborted!\r\n");
			return FALSE;
		}
		
		/* read one track */
		printf("Read   track %2d\r", track + 1);
		success = ReadTrack(src, track, buffer);
		if (!success)
		{
			printf("\r\nRead error drive %c track %d", src + 'A', track + 1);
			return FALSE;
		}
			
		/* write one track */
		printf("Write  track %2d\r", track + 1);
		success = WriteTrack(dest, track, buffer);
		if (!success)
		{
			stat = InPort(0xC0);
			
			printf("\r\nWrite error drive %c track %d",
				dest + 'A', track + 1, stat);
			if (stat & 0x40)
				printf("\r\nWrite protection");
			return FALSE;
		}
			
	} /* tracks */
	return TRUE;
}

/****************************************************************************/

BOOL Vfy2Drives(src, dest)
	int src, dest;
{
	int track, sector, side;
	int ch;
	unsigned int offset, b;
	BOOL success;
	int result;

	for (track = 0; track < TRACKS; track++)
	{
		ch = GetChr();
		if (ch == CTRLC) 
		{
			printf("\r\nCtrl-C detected. Aborted!\r\n");
			return FALSE;
		}
		
		/* read one track from src */
		printf("Read   track %2d\r", track + 1);
		success = ReadTrack(src, track, buffer);
		if (!success)
		{
			printf("\r\nRead error drive %c track %d", src + 'A', track + 1);
			return FALSE;
		}

		/* read one track from dest */
		printf("Verify track %2d\r", track + 1);
		success = ReadTrack(dest, track, vfy_buffer);
		if (!success)
		{
			printf("\r\nRead error drive %c track %d", dest + 'A', track + 1);
			return FALSE;
		}

		/* compare hole track */
		for (side = SIDE; side < SIDES; side++)
			for (sector = 0; sector < SECTORS; sector++)
			{
				offset = sector * (BYTES_PER_SECTOR * SIDES) + side * BYTES_PER_SECTOR;
				for (b = 0; b < BYTES_PER_SECTORS; b++)
				{
					if (*(vfy_buffer + offset + b) != *(buffer + offset + b))
					{
						printf("\r\nVerify error: track=%d side=%d sector=%d b=%u",
							track, side, sector, b);
						return FALSE;
					}
				}
			}
	} /* tracks */
	
	PutStr("\r\nVerify ok");
	return TRUE;
}

/****************************************************************************/
/* copy using 2 drives */

BOOL Copy2(src, dest, verify)
	int src, dest;
	BOOL verify;
{
	int ch;
	BOOL success;
	
	/* buffer for 1 track (2 sides) */
	buffer = CALLOC(SECTORS * BYTES_PER_SECTORS * SIDES, 1);
	if (buffer == NULL)
	{
		PutStr("Error: not enough memory\r\n");
		return FALSE;
	}

	if (verify)
	{
		vfy_buffer = CALLOC(SECTORS * BYTES_PER_SECTORS * SIDES, 1);
		if (vfy_buffer == NULL)
		{
			PutStr("Error: not enough memory\r\n");
			return FALSE;
		}
		/*printf("vfy_buffer = %u\r\n", vfy_buffer);*/
	}
	
	printf("Insert source disk in drive %c:\r\n", src + 'A');
	printf("Insert destination disk in drive %c:\r\n", dest + 'A');
	PutStr("Press ENTER to start!\r\n\n");
	ch = WaitCr();
	if (ch == CTRLC) return FALSE; /* ctrl-c */

	success = Copy2Drives(src, dest);
	if (!success) return FALSE;

	if (verify)
	{
		success = Vfy2Drives(src, dest);
		if (!success) return FALSE;
	}
	
	return TRUE;
}

/****************************************************************************/


main(argc, argv)
	int argc;
	char *argv[];
{
	int src, dest;
	BOOL argerr = FALSE;
	BOOL success;

	/*
	Test(0x1234, 0x5678, 0x9ABC);
	Flop(1, 0x24, 1, 2, 0x4567);
	*/

	BiosAddr();
	
	PutStr("\r\nDISKCOPY V1.0 for MC CP/M NDR/DD/SS/80T\r\n\n");

#ifndef DEBSRC
	if (argc != 3)
		argerr = TRUE;
	else
	{
		src = argv[1][0] - 'A';
		dest = argv[2][0] - 'A';
		
		argerr = strlen(argv[1]) != 2 || strlen(argv[2]) != 2 ||
				 src < 0 || src >= DRV_CNT || dest < 0 || dest >= DRV_CNT ||
				 argv[1][1] != ':' || argv[2][1] != ':';
	}
	
	if (argerr)
	{
		printf("Usage: copydisk src: dst: (src,dst = A to %c)\r\n", DRV_CNT - 1 + 'A');
		return;
	}
#else	
	src = DEBSRC;
	dest = DEBDEST;
#endif

	if (src == dest)
	{	/* using 1 drive */
		success = Copy1Drive(src);
	}
	else
	{	/* using 2 drives */
		success = Copy2(src, dest, TRUE);
	}

	PutStr("\r\n\nInsert SYSTEM-Disk and press ENTER\r\n");
	WaitCr();

	InitDisks();
	
	return;

}

/****************************************************************************/
