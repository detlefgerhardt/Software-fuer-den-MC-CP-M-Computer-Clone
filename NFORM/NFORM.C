/****************************************************************************/
/*
	NFORM.C  *dg*  07/2026 for MI-C compiler and MC CP/M clone
	
	Format disk, for FLO2 and special formats, Skew support.
	NFORM uses only Monitor routines and direct access to the floppy
	controller, so it can run without BIOS.
*/
/****************************************************************************/
/* NFORM version history

 28.07.2026 *dg* First version in C
 01.08.2026 *dg* added 1,44 MB format (1024 Byte/9 sectors/80 tracks)
			     renamed NDR format to NKC format
 03.08.2026 *dg* added 1,2 MB format (1024 Byte/7 sectors/80 tracks)
				 added more parametes
 04.08.2026 *dg* Formatierung repariert (side select Problem)
 
*/

#include "stdio.h"

#define FALSE 0
#define TRUE 1
#define WORD unsigned int
#define BOOL int

#define DDTZ FALSE

#define CTRLC 3
#define CR 13

/* for debuggng in old RunCPM CP/M 3.0 emulator */
BOOL isruncpm;

/* buffer for one track.
   one sector needs (sector-size + GAP3 + 62) bytes
   max. format is 1,44 MB with 9 sectors * 1024 bytes */
#define BUFFER_SIZE 1200 * 9
char *buffer;
char *skewbuf;
char outbuf[80];

extern int FRMTRK();
extern int VFYTRK();
extern HOME();

typedef struct
{
	char *Name;
	char SecCnt;	/* sectors per track (1..n) */
	int BytCnt;		/* sector byte count 128, 256, 512, 1024 */
	char TrkCnt;	/* track cnt (0..n-1) */
	char Density;	/* 0=SD, 1=DD */
	char SSDS;		/* 0 = single sided, 1 = double-sided */
	char MinMax;	/* 0 = Mini, 1 = Maxi */
	char UseSSO;	/* 1 = use SSO */
	char GapLen;	/* 54 = 3 ? */
	char Filler;	/* 0xE5 */
} format;

/* Format NKC Mini-Disk 800K */
format FMNKC =
{
	"NKC800",
	5,				/* 5 sectors */
	1024,			/* 1024 bytes per sector */
	80,				/* 80 tracks */
	1,				/* DD */
	1,				/* DS */
	0,				/* Mini (3.5/5.25 Zoll) */
	1,				/* UseSSO */
	54,				/* gap3 length */
	0xE5			/* filler */
};

/* Format 1,44 MB 3,5 Zoll HD/DS 1024/9 */
format HD35 =
{
	"HD3.5",
	9,				/* 9 sectors */
	1024,			/* 1024 bytes per sector */
	80,				/* 80 tracks */
	1,				/* DD */
	1,				/* DS */
	1,				/* Maxi or HD */
	1,				/* UseSSO */
	40,				/* gap3 length */
	0xE5			/* filler */
};

/* Format 1,4 MB 5,25 Zoll HD/DS 1024/7 */
format HD525 =
{
	"HD5.25A",
	8,				/* 8 sectors */
	1024,			/* 1024 bytes per sector */
	80,				/* 80 tracks */
	1,				/* DD */
	1,				/* DS */
	1,				/* Maxi or HD */
	1,				/* UseSSO */
	40,				/* gap3 length */
	0xE5			/* filler */
};

/* Format 1,4 MB 5,25 Zoll HD/DS 1024/7 */
format HD525B =
{
	"HD5.25B",
	15,				/* 15 sectors */
	512,			/* 512 bytes per sector */
	80,				/* 80 tracks */
	1,				/* DD */
	1,				/* DS */
	1,				/* Maxi or HD */
	1,				/* UseSSO */
	40,				/* gap3 length */
	0xE5			/* filler */
};

/* Format IBM 8" SD */
format FMIBMS =
{
	"IBM SS/SD",
	26,				/* 26 sectors */
	128,			/* 128 bytes per sector */
	77,				/* 77 tracks */
	0,				/* DD */
	0,				/* DS */
	1,				/* Maxi (8 Zoll) */
	0,				/* no UseSSO */
	27,				/* gap3 length */
	0xE5			/* filler */
};

#define FMTCNT 4
format *fmtlist[] = {FMNKC, HD35, HD525, HD525B, FMIBMS};

/****************************************************************************/

unsigned int BiosAddr()
{
#ASM
	LD HL,(0001H)	; BIOS address
	DEC HL
	DEC HL
	DEC HL
	;LD (BIOS),HL
#ENDASM
}

/****************************************************************************/

int ChkRunCpm()
{
	isruncpm = BiosAddr() == 0XEE00;
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
/* Monitor: read pyhsical sector */
/*
; HL = Buffer address
; E = Physical sector (1..n)
; D = Track (0..n-1)
; C = Drive-code + Density + Side (REG4-Latch)
; return 0 = no error
*/

#if FALSE

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

	CALL 0F021H	; Monitor FLOP routine
	LD L,A		; A=0: no error
	LD H,0
#ENDASM
}

#endif

/****************************************************************************/
/* returns !=0 if console char available */

int MConSt()
{
	#ASM
	CALL 0F012H		; Monitor console status, status in A (FFh if character available)
	LD L,A
	LD H,0
	#ENDASM	
}

/****************************************************************************/
/* Blocking Monitor console character in */

int MConIn()
{
	#ASM
	CALL 0F003H		; Monitor console in, character in A
	LD L,A
	LD H,0
	#ENDASM	
}

/****************************************************************************/
/* Monitor console character out */

MConOut(ch)
	int ch;
{
	#ASM
	POP BC
	POP HL
	PUSH HL
	PUSH BC
	LD C,L
	CALL 0F009H		; Monitor console out, character in C
	LD H,0
	#ENDASM	
}

/****************************************************************************/
/* non blocking BDOS console in, returns 0 if no char available */

int BConIn()
{
	return BDOS(0xFF, 6);
}

/****************************************************************************/
/* BDOS console out */

BConOut(ch)
	char ch;
{
	BDOS(ch, 6);
}

/****************************************************************************/
/* non blocking console character in, return 0 if no character available */

int GetChr()
{
	int ch;
	
	if (isruncpm)
	{
		/* BDOS */
		return BConIn();
	}
	else
	{
		/* Monitor */
		ch = MConSt();
		if (ch==0)
			return 0;	/* no char */
		else
			return MConIn();
	}
}

/****************************************************************************/
/* Console character out */

PutChr(ch)
	int ch;
{
	if (isruncpm)
		/* BDOS */
		BConOut(ch);
	else
		/* Monitor */
		MConOut(ch);
}		

/****************************************************************************/
/* Console string out, 0 terminated */

PutStr(s)
	char *s;
{
	while(*s)
		PutChr(*s++);
}

/****************************************************************************/
/* wait for CR or CTRL-C */

int WaitCr()
{
	int ch;
	
	while(TRUE)
	{
		ch = GetChr();
		if (ch == CTRLC || ch == CR) return ch; /* CR or Ctrl-C */
	}
}

/****************************************************************************/

/* start sector */
#define STSEC 1

BOOL CalcSkew(buf, seccnt, skew)
	char *buf;
	int seccnt, skew;
{
	int s, i, pos, sec;
	char *used;

	if (skew == 0 || skew == 1)
	{
		for (s = 0; s < seccnt; s++)
		{
			buf[s] = s + STSEC;
		}
		return TRUE;
	}
	
	used = calloc(seccnt, 1);
	if (used == NULL) return FALSE;

	for (s = 0; s < seccnt; s++)
		used[s] = 0;

	pos = 0;
	for(s = 0; s < seccnt; s++)
	{
		buf[pos] = s + STSEC;
		if (s == seccnt - 1) break;
		
		pos = (pos + skew) % seccnt;
		if (used[pos])
		{
			/* find next sector pos */
			for (i = 0; i < seccnt; i++)
			{
				if (!used[pos % seccnt]) break;
				pos++;
			}
		}
	}
	
	return TRUE;
}

/****************************************************************************/

ChkErr(err, track)
	int err, track;
{
	if (err == 0) return;

	/*sprintf(outbuf, "\r\ntrack %d code %02X:\r\n", track + 1, err);*/
	/*PutStr(outbuf);*/
	PutStr("\r\n");

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

ShowFmt(drive, fmt, skew)
	int drive, skew;
	format *fmt;
{
	char *str;
	
	/*str = "Format drive %c: %s [T=%d S=%d B=%d %s %s/%s I=%d %s]\r\n"; */
	sprintf(outbuf, "Format drive %c: %s [T=%d S=%d B=%d GAP3=%d ",
		drive+'A', fmt->Name, fmt->TrkCnt, fmt->SecCnt, fmt->BytCnt, fmt->GapLen);
	PutStr(outbuf);

	PutStr(fmt->MinMax == 0 ? "Mini" : "Maxi/HD");
	PutChr(' ');

	PutStr(fmt->SSDS == 0 ? "SS" : "DS");
	PutChr(' ');
	
	PutStr(fmt->Density == 0 ? "SD" : "DD");

	sprintf(outbuf, " I=%d]\r\n", skew);
	PutStr(outbuf);
}

/****************************************************************************/

ShowSkew(buf, seccnt)
	char *buf;
	int seccnt;
{
	int s;
	
	PutStr("Interleave: ");
	for (s = 0; s < seccnt; s++)
	{
		itoa(buf[s], outbuf);
		PutStr(outbuf);
		PutChr(' ');
	}
	PutStr("\r\n");
}

/****************************************************************************/

main(argc, argv)
	int argc;
	char *argv[];
{
	int p, track, stat, ch;
	int drive, skew, gaplen, seccnt, trkcnt, minmax, density;
	int heads, side, ctrl, addr; /* not implemented yet */
	int fmtidx;
	format *fmt;
	BOOL error, verify;
	
	ChkRunCpm();

	PutStr("\r\nNFORM 1.3 *dg* 26080-02\r\n");
	PutStr("Formatter for MC CP/M computer (FLO2)\r\n\n");

#if DDTZ == 0

	/* ----- parameter start ----- */

	fmtidx = 0;
	fmt = fmtlist[fmtidx];
	skew = 1;
	gaplen = -1;
	seccnt = -1;
	trkcnt = -1;
	minmax = -1;
	density = -1;
	heads = -1;
	side = -1;
	ctrl = -1;
	addr = 0xC0;
	verify = TRUE;

	error = FALSE;
	if (argc < 2)
		error = TRUE;
	
	if (!error && strlen(argv[1]) == 2 && argv[1][1] == ':')
	{
		drive = toupper(argv[1][0]) - 'A';
		if (drive < 0 || drive > 3)
			error = TRUE;
	}
	else
		error = TRUE;

	if (!error)
	{
		for (p = 2; p < argc; p++)
		{
			if (argv[p][0] != '-' && argv[p][0] != '/') continue;
			if (toupper(argv[p][1]) == 'F' && strlen(argv[p]) >= 2)
			{
				fmtidx = atoi(argv[p] + 2);
				if (fmtidx < 1 || fmtidx > FMTCNT)
					error= TRUE;
				fmtidx--;
				fmt = fmtlist[fmtidx];
			}
			if (toupper(argv[p][1]) == 'I' && strlen(argv[p]) >= 2 && fmt != NULL )
			{
				skew = atoi(argv[p] + 2);
				if (skew < 1 || skew > fmt->SecCnt)
					error= TRUE;
			}
			/* gaplen -G<g> */
			if (toupper(argv[p][1]) == 'G' && strlen(argv[p]) >= 2)
			{
				gaplen = atoi(argv[p] + 2);
				if (gaplen < 1 || gaplen > 100)
					error= TRUE;
			}
			/* seccnt -S<s> */
			if (toupper(argv[p][1]) == 'S' && strlen(argv[p]) >= 2)
			{
				seccnt = atoi(argv[p] + 2);
				if (seccnt < 1 || seccnt > 100)
					error= TRUE;
			}
			/* trkcnt -T<t> */
			if (toupper(argv[p][1]) == 'T' && strlen(argv[p]) >= 2)
			{
				trkcnt = atoi(argv[p] + 2);
				if (trkcnt < 1 || trkcnt > 100)
					error= TRUE;
			}
			/* mini/maxi -M<m> */
			if (toupper(argv[p][1]) == 'M' && strlen(argv[p]) >= 2)
			{
				minmax = atoi(argv[p] + 2);
				if (minmax != 0 && minmax != 1)
					error= TRUE;
			}
			/* density -D<s> */
			if (toupper(argv[p][1]) == 'D' && strlen(argv[p]) >= 2)
			{
				density = atoi(argv[p] + 2);
				if (density != 0 && density != 1)
					error= TRUE;
			}
			/* heads -H<h> */
			if (toupper(argv[p][1]) == 'D' && strlen(argv[p]) >= 2)
			{
				heads = atoi(argv[p] + 2);
				if (heads != 1 && heads != 2)
					error= TRUE;
			}
			/* side -H<h> */
			if (toupper(argv[p][1]) == 'D' && strlen(argv[p]) >= 2)
			{
				side = atoi(argv[p] + 2);
				if (side != 1 && side != 2)
					error= TRUE;
			}
			/*
			if (toupper(argv[p][1]) == 'V' && strlen(argv[p]) == 2)
				verify = TRUE;
			*/
		}
	}
	
	if (error)
	{
		PutStr("usage: NFORM <d>: -F<f> -I<i> -S<s> -T<t> -G<g> -M<m> -D<d>\r\n");
		PutStr("  d = Drive A..D\r\n");
		PutStr("  f = Format 1=NKC 800KB, 2=1,44MB, 3=5.25(1K) 4=5.25(512) 5=IBM SS/SD (default=1)\r\n");
		PutStr("  i = Interleave/Skew (default=1)\r\n");
		PutStr("  s = Sector count\r\n");
		PutStr("  g = Gap3\r\n");
		PutStr("  t = Track count\r\n");
		PutStr("  m = 0=Mini/SD, 1=Maxi/HD\r\n");
		PutStr("  d = Density (0=SD, 1=DD)\r\n");
		/*
		PutStr("  h = Heads (1=SS, 2=DS)\r\n");
		PutStr("  s = Side 1/2 (only with SS)\r\n");
		*/
		/*PutStr("  V = Verify\r\n");*/
		return;
	}

#else
	drive = 2;
	fmtidx = 0;
	fmt = fmtlist[fmtidx];
	skew = 1;
	gaplen = -1;
	seccnt = -1;
	trkcnt = -1;
	minmax = -1;
	density = -1;
	heads = -1;
	side = -1;
	verify = TRUE;
#endif


	if (seccnt > 0)
	{
		sprintf(outbuf, "sectors=%d\r\n", seccnt);
		PutStr(outbuf);
		fmt->SecCnt = seccnt;
	}
	if (gaplen > 0)
	{
		sprintf(outbuf, "gap3=%d\r\n", gaplen);
		PutStr(outbuf);
		fmt->GapLen = gaplen;
	}
	if (trkcnt > 0)
	{
		sprintf(outbuf, "tracks=%d\r\n", trkcnt);
		PutStr(outbuf);
		fmt->TrkCnt = trkcnt;
	}
	if (minmax > 0)
	{
		sprintf(outbuf, "mini/maxi=%d\r\n", minmax);
		PutStr(outbuf);
		fmt->MinMax = minmax;
	}
	if (density > 0)
	{
		sprintf(outbuf, "density=%d\r\n", density);
		PutStr(outbuf);
		fmt->Density = density;
	}

	skewbuf = calloc(fmt->SecCnt, 1);
	buffer = calloc(BUFFER_SIZE, 1);
	if (buffer == NULL || skewbuf == NULL)
	{
		PutStr("Not enough memory\r\n");
		return;
	}

	/* ----- parameter end ----- */

	CalcSkew(skewbuf, fmt->SecCnt, skew);
	ShowFmt(drive, fmt, skew, gaplen);
	/*if (skew > 1)*/
		ShowSkew(skewbuf, fmt->SecCnt);
	
	sprintf(outbuf, "\r\nInsert disk in drive %c: and press ENTER\r\n\r\n", drive + 'A');
	PutStr(outbuf);
	
	ch = WaitCr();
	if (ch == 3) return;

	trkcnt = fmt->TrkCnt;
	
	INIT(buffer, drive, skewbuf, fmt);
	HOME();
	error = FALSE;
	for (track = 0; track < trkcnt; track++)
	{
		ch = GetChr();
		if (ch == CTRLC)
		{
			PutStr("\r\nAborted\r\n");
			return;
		}
		
		stat = FRMTRK(track);
		sprintf(outbuf, "Track %02d Format (%02x)\r", track + 1, stat);
		PutStr(outbuf); 
		if (stat != 0)
		{
			ChkErr(stat, track);
			error = TRUE;
			break;
		}

		if (verify)
		{
			
			stat = VFYTRK(track);
			sprintf(outbuf, "Track %02d Verify (%02x)\r", track + 1, stat);
			PutStr(outbuf);
			if (stat !=0 )
			{
				ChkErr(stat, track);
				error = TRUE;
				break;
			}
		}
		if (track < trkcnt - 1)
			STEPIN();
	}

	/*
	if (verify && !error)
	{
		HOME();
		error = FALSE;
		for (track = 0; track < trkcnt; track++)
		{
			ch = GetChr();
			if (ch == CTRLC)
			{
				PutStr("\r\nAborted\r\n");
				return;
			}
			
			stat = VFYTRK(track);
			sprintf(outbuf, "Verify track %02d (%02x)\r", track + 1, stat);
			PutStr(outbuf);
			if (stat !=0 )
			{
				ChkErr(stat, track);
				error = TRUE;
				break;
			}

			if (track < trkcnt - 1)
				STEPIN();
		}
	}
	*/

	if (!error)
		PutStr("\r\nfinished\r\n");
}

/****************************************************************************/
