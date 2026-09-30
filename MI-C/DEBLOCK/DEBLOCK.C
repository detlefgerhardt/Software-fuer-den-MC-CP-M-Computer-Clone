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
 05.09.2026 *dg* Used to test new IDE/CF format (8192 KB)
 
 */

#include "stdio.h"

#define FALSE 0
#define TRUE 1
#define WORD unsigned int
#define BOOL int

#define CTRLC 3
#define CR 13

#define DRV_CNT 8 /* number of drives */

/*#define DEBDRV 4*/

char outbuf[120];

char *secbuf;

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
	SetTrk(track);
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
	SetTrk(track);
	SetSec(sector);
	SetDma(addr);
	return BiosWrite();  /* write sector by BIOS */
}

/****************************************************************************/

BOOL WriteDisk(drive, seccnt, trkcnt, sidcnt, debid, buffer)
	int drive, seccnt, trkcnt, sidcnt, debid;
	char *buffer;
{
	int track, trk, side, sector, b;
	int ch, stat, result;

	for (track = 0; track < trkcnt; track++)
	{
		ch = GetChr();
		if (ch == CTRLC) 
		{
			printf("\r\nCtrl-C detected. Aborted!\r\n");
			return FALSE;
		}
		if (ch == ' ')
		{
			WaitCr();
		}

		for (side = 0; side < sidcnt; side++)
		{
			printf("Write track=%3d side=%d\r", track, side);

			for (sector = 0; sector < seccnt; sector++)
			{
				buffer[0] = track;
				buffer[1] = side;
				buffer[2] = sector;
				buffer[3] = debid;
				trk = track * sidcnt + side;
				result = WriteSector(drive, trk, side, sector, buffer);

				if (result != 0)
				{
					stat = InPort(0xC0);
				
					printf("\r\nWrite error drive %c track %d side=%d sector=%d result=%02X stat=%02X\r\n",
						drive + 'A', track, side, sector, result, stat);
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

BOOL ReadDisk(drive, seccnt, trkcnt, sidcnt, debid, buffer)
	int drive, seccnt, trkcnt, sidcnt, debid;
	char *buffer;
{
	int track, trk, side, sector, b, errcnt;
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

		for (side = 0; side < sidcnt; side++)
		{
			printf("Read  track=%3d side=%d\r", track, side);

			for (sector = 0; sector < seccnt; sector++)
			{
				trk = track * sidcnt + side;
				result = ReadSector(drive, trk, side, sector, buffer);
				if (result != 0)
				{
					stat = InPort(0xC0);

					printf("\r\nRead error drive=%c track=%d side=%d sector=%d result=%02X stat=%02X\r\n",
						drive + 'A', track, side, sector, result, stat);
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
	int drive, seccnt, trkcnt, secsize, sidcnt, b, p, ch;
	BOOL error, success;

	BiosAddr();
	
	PutStr("\r\nDEBLOCK V1.1 *dg* 260807-03\r\n");
	PutStr("deblocking test program\r\n\n");

	seccnt = 72;
	trkcnt = 80;
	secsize = 128;
	sidcnt = 2;
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
			if (toupper(argv[p][1]) == 'F' && strlen(argv[p]) >= 2)
			{
				if (toupper(argv[p][2]) == 'I')
				{
					/* IDE 8192 */
					seccnt = 256;
					trkcnt = 256;
					secsize = 128;
					sidcnt = 1;
				}
				else if (toupper(argv[p][2]) == 'N')
				{
					/* NKC 800 format */
					seccnt = 72;
					trkcnt = 80;
					secsize = 128;
					sidcnt = 2;
				}
			}
			
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
			if (toupper(argv[p][1]) == 'C' && strlen(argv[p]) >= 2)
			{
				sidcnt = atoi(argv[p] + 2);
				if (sidcnt < 1 || sidcnt > 2)
					error = TRUE;
			}
		}
	}
	
	if (error || drive == -1)
	{
		/*printf("%d %d %d %d\r\n", drive, seccnt, secsize, trkcnt);*/
		PutStr("Usage: deblock <d>: -F<f> -S<s> -Z<z> -T<t> -C<c>\r\n");
		PutStr("  d = Drive A..H\r\n");
		PutStr("  f = Format [N]KC / [I]DE\r\n");
		PutStr("  s = Sector count [1..256]\r\n");
		PutStr("  z = Sector size [1..1024]\r\n");
		PutStr("  t = Track count [1-256]\r\n");
		PutStr("  c = Side count [2]\r\n");
		return;
	}
#else	
	drive = DEBDRV;
	seccnt = 256;
	trkcnt = 10;
	secsize = 128;
	sidcnt = 1;
#endif

	secbuf = CALLOC(secsize, 1);
	if (secbuf == NULL)
	{
		PutStr("Memory error\r\n");
		return FALSE;
	}

	sprintf(outbuf, "Drive        : %c\r\n", drive + 'A');
	PutStr(outbuf);
	sprintf(outbuf, "Bytes/Sector : %d\r\n", secsize);
	PutStr(outbuf);
	sprintf(outbuf, "Sectors/Track: %d\r\n", seccnt);
	PutStr(outbuf);
	sprintf(outbuf, "Tracks       : %d\r\n", trkcnt);
	PutStr(outbuf);
	sprintf(outbuf, "Sides        : %d\r\n", sidcnt);
	PutStr(outbuf);

	sprintf(outbuf, "\r\nInsert disk in drive %c: and press ENTER\r\n", drive + 'A');
	PutStr(outbuf);
	sprintf(outbuf, "Warning! All data on disk will be overwritten!\r\n\r\n");
	PutStr(outbuf);
	ch = WaitCr();
	if (ch == CTRLC) return;

	/*InitDisks();*/

	for (b = 0; b < secsize; b++)
		secbuf[b] = 0xE5;

	/*
	DebDisk(secbuf);
	return;
	*/

	success = WriteDisk(drive, seccnt, trkcnt, sidcnt, 1, secbuf);
	
	if (success)
	{
		ReadDisk(drive, seccnt, trkcnt, sidcnt, 1, secbuf);
	}

	PutStr("\r\nInsert SYSTEM-Disk and press ENTER\r\n");
	WaitCr();

	/*
	InitDisks();
	*/
}

/****************************************************************************/
