/****************************************************************************/
/*
	DISKCOPY.C  *dg*  09/2026 for MI-C compiler and MC CP/M clone
	
	Copy a disk from one drive to another drive or on one drive.
	Only for format: NDR - DD/DS/80 track/800 KB.
*/
/****************************************************************************/
/* DISKCOPY version history

 23.07.2026 *dg* First version
 30.09.2026 *dg* Use NBIOS function GETPHY to get phys. drive parameters
 
 */

#include "stdio.h"

#define FALSE 0
#define TRUE 1
#define WORD unsigned int
#define BOOL int

#define CTRLC 3
#define CR 13

/*#define USEMON TRUE*/
/*
#define DEBSRC 0
#define DEBDEST 1
*/

/* format definiion */
/*#define TRACKS 80

#if USEMON == FALSE

#define BYTES_PER_SECTOR 128
#define SECTORS 40

#else

#define BYTES_PER_SECTOR 1024
#define SECTORS 5

#endif

#define SIDES 2
*/
/*#define SIDE 0*/ /* side if SS */


#define DRV_CNT 8 /* number of drives */

char *buffer;
char *vfy_buffer;

typedef struct
{
	int drive;
	int byts;
	char secs;
	char trks;
	char sids;
	char side; /* side to use if only one side */
	int size; /* in KB */
} drvprm;


#ASM
MFLOP	EQU		0F021H	; Monior FLOP routine

BIOS:	DW	0			; BIOS addr for faster access
#ENDASM

/****************************************************************************/
/* set BIOS addr for faster access */

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
/* BIOS read sector from disk */
/* call BiosAddr to initialise BIOS start address !!! */

#IF FALSE
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
#ENDIF

/****************************************************************************/
/* BDOS init disk system */

InitDisks()
{
	BDOS(0, 0x0D);
}

/****************************************************************************/
/* BIOS write sector to disk */

#IF FALSE
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
#ENDIF

/****************************************************************************/
/* Monitor: read pyhsical sector */
/*
; HL = Buffer address
; E = Physical sector (1..n)
; D = Track (0..n-1)
; C = Drive-code + Density + Side (REG4-Latch)
; return 0 = no error
*/

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

	CALL MFLOP	; Monitor FLOP routine
	LD L,A		; A=0: no error
	LD H,0
#ENDASM
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

int ReadSector(drive, track, side, sector, addr)
	int drive, track, side, sector;
	unsigned addr;
{
	int result;
	
	int drvcod = (1 << drive) | 0x20;
	if (side == 1) drvcod |= 0x80;
	 /* read sector by Monitor */
	return Flop(1, drvcod, track, sector + 1, addr);
}

/****************************************************************************/
/* drive 0..n-1 */
/* track 0..n-1 */
/* side 0..n.1 */
/* sector 0..n-1 */
/* addr = buffer address */

#if FALSE

int WriteSector(drive, track, side, sector, addr)
	int drive, track, side, sector;
	unsigned addr;
{
	int result;
	/*cprintf("\r\nWR %d %d %d %d %u\t\n", drive, track, side, sector, addr);*/
	
	int drvcod = (1 << drive) | 0x20;
	if (side == 1) drvcod |= 0x80;
	 /* write sector by Monitor */
	return Flop(2, drvcod, track, sector + 1, addr); /* write sector */
}

#ENDIF

/****************************************************************************/
/* read one track */

BOOL ReadTrack(drive, track, prm, buffer)
	int drive, track;
	drvprm *prm;
	unsigned buffer;
{
	unsigned offset;
	int side, sector, drvcod;
	int result;

	for (side = prm->side; side < prm->sids; side++)
		for (sector = 0; sector < prm->secs; sector++)
		{
		
			drvcod = (1 << drive) | 0x20;
			if (side == 1) drvcod |= 0x80;
			offset = sector * (prm->byts * prm->sids) + side * prm->byts;
			 /* read sector by Monitor */
			result = Flop(1, drvcod, track, sector + 1, buffer + offset);
			if (result != 0) return FALSE;
		}
	return TRUE;
}

/****************************************************************************/
/* write one track */

BOOL WriteTrack(drive, track, prm, buffer)
	int drive, track;
	drvprm *prm;
	unsigned buffer;
{
	unsigned offset;
	int side, sector, drvcod;
	int result;

	for (side = prm->side; side < prm->sids; side++)
		for (sector = 0; sector < prm->secs; sector++)
		{
			drvcod = (1 << drive) | 0x20;
			if (side == 1) drvcod |= 0x80;
			offset = sector * (prm->byts * prm->sids) + side * prm->byts;
			 /* write sector by Monitor */
			result = Flop(2, drvcod, track, sector + 1, buffer + offset); /* write sector */
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

	cputs("Copy on one drive not implemented yet");
	return FALSE;

#if FALSE

	tracksize = prm SECTORS * BYTES_PER_SECTORS * SIDES;
	/*cprintf("tracksize=%d\r\n", tracksize);*/
	
	for (tracks = 5; tracks > 0; tracks--)
	{
		bufsize = tracks * tracksize;
		buffer = CALLOC(SECTORS * BYTES_PER_SECTORS * SIDES, 1);
		if (buffer != NULL) break;
	}
	
	if (tracks == 0)
	{
		cputs("Error: not enough memory\r\n");
		return FALSE;
	}
	
	cprintf("tracks = %d, changes = %d\r\n", tracks, TRACKS / tracks + 1);
#endif
}

/****************************************************************************/

BOOL Copy2Drives(src, dest, prm)
	int src, dest;
	drvprm *prm;
{
	int track, ch, stat;
	BOOL success;

	for (track = 0; track < prm->trks; track++)
	{
		if (CtrlC()) return FALSE;
		
		/* read one track */
		cprintf("Read   track %2d\r", track + 1);
		success = ReadTrack(src, track, prm, buffer);
		if (!success)
		{
			cprintf("\r\nRead error drive %c track %d", src + 'A', track + 1);
			return FALSE;
		}
			
		/* write one track */
		cprintf("Write  track %2d\r", track + 1);
		success = WriteTrack(dest, track, prm, buffer);
		if (!success)
		{
			stat = InPort(0xC0);
			
			cprintf("\r\nWrite error drive %c track %d",
				dest + 'A', track + 1, stat);
			if (stat & 0x40)
				cprintf("\r\nWrite protection");
			return FALSE;
		}
			
	} /* tracks */
	return TRUE;
}

/****************************************************************************/

BOOL Vfy2Drives(src, dest, prm)
	int src, dest;
	drvprm *prm;
{
	int track, sector, side;
	int ch;
	unsigned int offset, b;
	BOOL success;
	int result;

	for (track = 0; track < prm->trks; track++)
	{
		if (CtrlC()) return FALSE;
		
		/* read one track from src */
		cprintf("Read   track %2d\r", track + 1);
		success = ReadTrack(src, track, prm, buffer);
		if (!success)
		{
			cprintf("\r\nRead error drive %c track %d", src + 'A', track + 1);
			return FALSE;
		}

		/* read one track from dest */
		cprintf("Verify track %2d\r", track + 1);
		success = ReadTrack(dest, track, prm, vfy_buffer);
		if (!success)
		{
			cprintf("\r\nRead error drive %c track %d", dest + 'A', track + 1);
			return FALSE;
		}

		/* compare hole track */
		for (side = prm->side; side < prm->sids; side++)
			for (sector = 0; sector < prm->secs; sector++)
			{
				offset = sector * (prm->byts * prm->sids) + side * prm->byts;
				for (b = 0; b < prm->byts; b++)
				{
					if (*(vfy_buffer + offset + b) != *(buffer + offset + b))
					{
						cprintf("\r\nVerify error: track=%d side=%d sector=%d b=%u",
							track, side, sector, b);
						return FALSE;
					}
				}
			}
	} /* tracks */
	
	cputs("\r\nVerify ok");
	return TRUE;
}

/****************************************************************************/
/* copy using 2 drives */

BOOL Copy2(src, dest, prm, verify)
	int src, dest;
	drvprm *prm;
	BOOL verify;
{
	int ch, bufsiz;
	BOOL success;

	bufsiz = prm->byts * prm->secs * prm->sids;
	
	/* buffer for 1 track (2 sides) */
	buffer = CALLOC(bufsiz, 1);
	if (buffer == NULL)
	{
		cprintf("Error: not enough memory (%d)\r\n", bufsiz);
		return FALSE;
	}

	if (verify)
	{
		vfy_buffer = CALLOC(bufsiz, 1);
		if (vfy_buffer == NULL)
		{
			cprintf("Error: not enough memory (%d)\r\n", bufsiz);
			return FALSE;
		}
		/*cprintf("vfy_buffer = %u\r\n", vfy_buffer);*/
	}
	
	cprintf("\r\nInsert source disk in drive %c:\r\n", src + 'A');
	cprintf("Insert destination disk in drive %c:\r\n", dest + 'A');
	cputs("Press ENTER to start!\r\n\n");
	ch = WaitCr();
	if (ch == CTRLC) return FALSE; /* ctrl-c */

	success = Copy2Drives(src, dest, prm);
	if (!success) return FALSE;

	if (verify)
	{
		success = Vfy2Drives(src, dest, prm);
		if (!success) return FALSE;
	}
	
	return TRUE;
}

/****************************************************************************/
/* *prm is call by reference */

GetPrm(drive, prm)
	int drive;
	drvprm *prm;
{
	int curdrv;

	curdrv = GetDrv();
	SelDrv(drive);

	prm->drive = drive;
	prm->byts = GetByt();
	prm->secs = GetSec();
	prm->trks = GetTrk();
	prm->sids = GetSid();
	prm->side = 0; /* not used */
	prm->size = (int)((long)prm->byts * prm->secs * prm->trks * prm->sids / 1024);

	SelDrv(curdrv);
}

/****************************************************************************/

BOOL cmpprm(src, dst)
	drvprm *src, *dst;
{
	if (src->byts != dst->byts) return FALSE;
	if (src->secs != dst->secs) return FALSE;
	if (src->trks != dst->trks) return FALSE;
	if (src->sids != dst->sids) return FALSE;
	return TRUE;
}

/****************************************************************************/

main(argc, argv)
	int argc;
	char *argv[];
{
	drvprm srcprm, dstprm;
	int src, dest, p;
	BOOL verify;
	BOOL error;
	BOOL success;

	BiosAddr();
	
	cputs("\r\nDISKCOPY V1.1 for MC CP/M *dg* 260930-01\r\n\n");

	verify = FALSE;
	error = FALSE;
	if (argc < 3)
		error = TRUE;
	else
	{
		src = argv[1][0] - 'A';
		dest = argv[2][0] - 'A';
		
		error = strlen(argv[1]) != 2 || strlen(argv[2]) != 2 ||
				src < 0 || src >= DRV_CNT || dest < 0 || dest >= DRV_CNT ||
				argv[1][1] != ':' || argv[2][1] != ':';
	}
	
	if (!error)
	{
		for (p = 2; p < argc; p++)
		{
			if (argv[p][0] != '-' && argv[p][0] != '/') continue;
			if (toupper(argv[p][1]) == 'V' && strlen(argv[p]) >= 2)
			{
				verify = TRUE;
			}
		}
	}

	if (error)
	{
		cprintf("Usage: diskcopy src: dst: (src,dst = A to %c) -v\r\n", DRV_CNT - 1 + 'A');
		cputs  ("  -v = verify\r\n");
		return;
	}

	GetPrm(src, &srcprm);
	GetPrm(dest, &dstprm);

	cprintf("src %c: (%d/%d/%d/%d/%dK)\r\n", src+'A', srcprm.byts, srcprm.secs, srcprm.trks, srcprm.sids, srcprm.size);
	cprintf("dst %c: (%d/%d/%d/%d/%dK)\r\n", dest+'A', dstprm.byts, dstprm.secs, dstprm.trks, dstprm.sids, dstprm.size);
	if (verify)
	{
		cputs("verify = on\r\n");
	}

	if (!cmpprm(&srcprm, &dstprm))
	{
		cprintf("Formats of drive %c and %c do not match!\r\n", src+'A', dest+'A');
		EXIT();
	}
	
	if (src == dest)
	{	/* using 1 drive */
		success = Copy1Drive(src);
	}
	else
	{	/* using 2 drives */
		success = Copy2(src, dest, srcprm, verify);
	}

	cputs("\r\n\nInsert SYSTEM-Disk and press ENTER\r\n");
	WaitCr();

	/*InitDisks();
	return; */

	EXIT();
}

/****************************************************************************/
