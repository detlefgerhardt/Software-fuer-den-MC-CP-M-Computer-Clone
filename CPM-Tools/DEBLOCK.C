/****************************************************************************/
/*
	DEBLOCK.C  *dg*  08/2026 for MI-C compiler and MC CP/M clone
	
	Write individual pattern to each logical sector and read it back.
	Blocking/Deblocking test
*/
/****************************************************************************/
/* DEBLOCK version history

 03.08.2026 *dg* First version
 07.08.2026 *dg* tracks, sectors and sector count as parameters
                 renamed to DEBLOCK.C
 
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
/*#define MSECTOR_SIZE 1024*/

#define DRV_CNT 8 /* number of drives */

/*#define DEBDRV 1
#define DEBSEC 56*/

char outbuf[120];

char *secbuf;
/*char *mbuffer;*/

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

BOOL Flop(cmd, drive, track, sector, addr)
	int cmd,drive, track, sector;
	unsigned addr;
{
#ASM
	LD IX,2			; RET
	ADD IX,SP
	LD B,(IX+0)		; cmd (1= read, 2=write)
	LD C,(IX+2)		; drive code
	LD D,(IX+4)		; track
	LD E,(IX+6)		; sector
	LD L,(IX+8)		; addr (high)
	LD H,(IX+9)		; addr (low)
	CALL 0F021H		; Monitor FLOP routine
	LD L,A			; A=0: no error
	LD H,0
#ENDASM
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

BOOL WriteDisk(drive, seccnt, trkcnt, debid, buffer)
	int drive, seccnt, trkcnt, debid;
	char *buffer;
{
	int track, side, sector, b;
	int ch, stat, result;

	for (track = 0; track < trkcnt; track++)
	{
		ch = GetChr();
		if (ch == CTRLC) 
		{
			printf("\r\nCtrl-C detected. Aborted!\r\n");
			return FALSE;
		}

		for (side = 0; side < SIDES; side++)
		{
			printf("Write track=%d side=%d\r", track, side);

			for (sector = 0; sector < seccnt; sector++)
			{
				buffer[0] = track;
				buffer[1] = side;
				buffer[2] = sector;
				buffer[3] = debid;
				result = WriteSector(drive, track, side, sector, buffer);

				if (result != 0)
				{
					stat = InPort(0xC0);
				
					printf("\r\nWrite error drive %c track %d side=%d sector=%d stat=%02X\r\n",
						drive + 'A', track, side, sector, stat);
					if (stat & 0x40)
						printf("\r\nWrite protection\r\n");
					return FALSE;
				}
			}
		}
	} /* tracks */
	
	PutStr("\r\n");
	return TRUE;
}

/****************************************************************************/

BOOL ReadDisk(drive, seccnt, trkcnt, debid, buffer)
	int drive, seccnt, trkcnt, debid;
	char *buffer;
{
	int track, side, sector, b, errcnt;
	int ch, stat, result;
	BOOL error;

	errcnt = 0;

	for (track = 0; track < trkcnt; track++)
	{
		ch = GetChr();
		if (ch == CTRLC) 
		{
			printf("\r\nCtrl-C detected. Aborted!\r\n");
			return FALSE;
		}

		for (side = 0; side < SIDES; side++)
		{
			printf("Read  track=%d side=%d\r", track, side);

			for (sector = 0; sector < seccnt; sector++)
			{
				result = ReadSector(drive, track, side, sector, buffer);
				if (result != 0)
				{
					stat = InPort(0xC0);

					printf("\r\nRead error drive=%c track=%d side=%d sector=%d stat=%02X\r\n",
						drive + 'A', track, side, sector, stat);
					return FALSE;
				}

				error = FALSE;
				if (buffer[0] != track || buffer[1] != side || buffer[2] != sector || buffer[3] != debid)
				{
					error = TRUE;
					printf("\r\ntrack error trk=%d/%d sid=%d/%d sec=%d/%d id=%d/%d\r\n", 
						track, buffer[0], side, buffer[1], sector, buffer[2], debid, buffer[3]);
				}
				if (error)
				{
					errcnt++;
					if (errcnt > 5) return FALSE;
					/*
					ch = WaitCr();
					if (ch==CTRLC) return FALSE;
					*/
				}
			}
		}
	} /* tracks */
	
	PutStr("\r\n");
	
	return TRUE;
}

/****************************************************************************/

#if FALSE

BOOL MReadDisk(drive, seccnt, buffer)
	int drive, seccnt;
	char *buffer;
{
	int track, side, sector, b, drvcod;
	int ch, stat, result;
	unsigned offset;
	BOOL error;

	/*for (track = 0; track < TRACKS; track++)*/
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
			for (sector = 1; sector <= seccnt; sector++)
			{
				printf("MRead track=%d side=%d sector=%d %04X\r\n", track, side, sector, buffer);

				/*int drvcod = (1 << drive) | 0x20;*/
				drvcod = (1 << drive);
				if (side == 1) drvcod |= 0x80;
				/* read sector by Monitor */
				result = Flop(1, drvcod, track, sector, buffer);
				if (result != 0)
				{
					stat = InPort(0xC0);
				
					printf("\r\nRead error drive %c track %d side=%d sector=%d stat=%02X\r\n",
						drive + 'A', track, side, sector, stat);
					return FALSE;
				}

				for (b = 0; b < 8; b++)
				{
					offset = b * 128;
					printf("sec=%d ofs=%04X [%d %d %d]\r\n", sector, offset, buffer[offset], buffer[offset+1], buffer[offset+2]);
				}

				/*
				if (error)
				{
					ch = WaitCr();
					if (ch==CTRLC) return FALSE;
				}
				*/
			}
		}
	} /* tracks */
	return TRUE;
}

/****************************************************************************/

int DebWrit(drive, track, side, sector, debid, buffer)
	int drive, track, side, sector, debid;
	char *buffer;
{
	int result;
	
	buffer[0] = track;
	buffer[1] = side;
	buffer[2] = sector;
	buffer[3] = debid;
	printf("Write sector track=%d side=%d sector=%d id=%d\r\n", track, side, sector, debid);
	result = WriteSector(drive, track, side, sector, buffer);
	if (result != 0)
	{
		ChkErr();
	}
	return result;
}

/****************************************************************************/

int DebRead(drive, track, side, sector, buffer)
	int drive, track, side, sector;
	char *buffer;
{
	int result;
	
	printf("Read sector track=%d side=%d sector=%d\r\n", track, side, sector);
	result = ReadSector(drive, track, side, sector, buffer);
	printf("D=%d T=%d S=%d ID=%d\r\n", buffer[0], buffer[1], buffer[2], buffer[3]);
	if (result != 0)
	{
		ChkErr();
	}
	return result;
}

/****************************************************************************/

int DebDisk(buffer)
	char *buffer;
{
	int result, drive, track, side, sector, debid;

	debid = 1;
	drive = 1;
	
	track = 0;
	side = 1;
	/*
	for (sector = 48; sector < 56; sector++)
	{
		result = DebWrit(drive, track, side, sector, buffer);
	}
	*/

	track = 1;
	side = 0;
	for (sector = 0; sector < 8; sector++)
	{
		result = DebWrit(drive, track, side, sector, debid, buffer);
	}

	result = DebWrit(drive, track, side, 12, debid, buffer);

	
	track = 1;
	side = 0;
	for (sector = 0; sector < 8; sector++)
	{
		result = DebRead(drive, track, side, sector, buffer);
		if (result !=0 || buffer[0] != track || buffer[1] != side || buffer[2] != sector)
		{
			ChkErr();
			printf("\r\nerror\r\n");
			return;
		}
	}
}

#endif

/****************************************************************************/

ChkErr()
{
	int err;
	
	err = InPort(0xC0);

	if (err == 0) return;

	if (err & 0x04)
		PutStr("CPU to slow error\r\n");
	
	if (err & 0x08)
		PutStr("CRC error\r\n");

	if (err & 0x10)
		PutStr("Record not found error\r\n");
	
	if (err & 0x20)
		PutStr("Wrong record type error\r\n");

	if (err & 0x40)
		PutStr("Write protect error\r\n");
	
	if (err & 0x80)
		PutStr("Unknown error\r\n");
}

/****************************************************************************/

main(argc, argv)
	int argc;
	char *argv[];
{
	int drive, seccnt, trkcnt, secsize, b, p, ch;
	BOOL error, success;

	BiosAddr();
	
	PutStr("\r\nDEBLOCK V1.1 *dg* 260807-03\r\n\n");

	seccnt = 72;
	trkcnt = 80;
	secsize = 1024;
	drive = -1;
	

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
			if (toupper(argv[p][1]) == 'Z' && strlen(argv[p]) >= 2)
			{
				secsize = atoi(argv[p] + 2);
				if (secsize < 128 || secsize > 1024)
					error = TRUE;
			}
			if (toupper(argv[p][1]) == 'T' && strlen(argv[p]) >= 2)
			{
				trkcnt = atoi(argv[p] + 2);
				if (trkcnt< 1 || trkcnt > 255)
					error = TRUE;
			}
		}
	}
	
	if (error)
	{
		printf("%d %d %d %d\r\n", drive, seccnt, secsize, trkcnt);
		PutStr("Usage: deblock d: -S<s> -Z<z> -T<t>\r\n");
		PutStr("  d = Drive A..D\r\n");
		PutStr("  s = Sector count\r\n");
		PutStr("  z = Sector size\r\n");
		PutStr("  t = Track count\r\n");
		return;
	}
#else	
	drive = DEBDRV;
	seccnt = 60;
	trkcnt = 80;
	secsize = 128
#endif

	secbuf = CALLOC(secsize, 1);
	if (secbuf == NULL)
	{
		PutStr("Error: not enough memory\r\n");
		return FALSE;
	}
	/*
	mbuffer = CALLOC(MSECTOR_SIZE, 1);
	if (buffer == NULL)
	{
		PutStr("Error: not enough memory\r\n");
		return FALSE;
	}
	*/

	sprintf(outbuf, "sectors = %d, size=%d, tracks=%d\r\n", seccnt, secsize, trkcnt);
	PutStr(outbuf);

	sprintf(outbuf, "\r\nInsert disk in drive %c: an press ENTER\r\n", drive + 'A');
	PutStr(outbuf);
	sprintf(outbuf, "Warning! All data on disk will be overwritten!\r\n\r\n");
	PutStr(outbuf);
	ch = WaitCr();
	if (ch == CTRLC) return;

	/*InitDisks();*/

	for (b = 0; b < secsize; b++)
		secbuf[b] = 0x55;

	/*
	DebDisk(secbuf);
	return;
	*/

	success = WriteDisk(drive, seccnt, trkcnt, 1, secbuf);
	
	if (success)
	{
		ReadDisk(drive, seccnt, trkcnt, 1, secbuf);
	}

	PutStr("\r\nInsert SYSTEM-Disk and press ENTER\r\n");
	WaitCr();

	/*
	InitDisks();
	*/
}

/****************************************************************************/
