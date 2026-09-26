/****************************************************************************/
/*
	CHGDRV.C  *dg*  09/2026 for MI-C compiler and MC CP/M clone
	
	Changes drive parameters at runtime
*/
/****************************************************************************/
/* CHGDRV version history

 25.09.2026 *dg* First version
 
 */

#include "stdio.h"

#define FALSE 0
#define TRUE 1
#define WORD unsigned int
#define BOOL int

#define CTRLC 3
#define CR 13

#define DRVCNT 8 /* number of drives */

/****************************************************************************/

typedef struct
{
	unsigned spt;	/* sectors per track */
	char bsh;		/* block shift */
	char blm;		/* block mask */
	char exm;		/* extnt mask */
	unsigned dsm;	/* disk size - 1 */
	unsigned drm;	/* dir max */
	char al0;		/* alloc0 */
	char al1;		/* alloc1 */
	unsigned cks;	/* checked dir size */
	unsigned ofs;	/* track offset */
	/* blocking/deblocking */
	char psh;		/* physical shift */
	char phm;		/* physical mask */
	/* FLO register */
	char reg;		/* register */
	/* physical params */
	unsigned phylen; /* phys. bytes/sector */
	char physec;	/* phys. sectors/count */
	char phytrk;	/* phys. tracks/side */
	char physid;	/* phys. sides */
} drvtab;

typedef struct
{
	char *name;
	int size; /* size in KB */
	drvtab tab;
} drvprm;

#include "NBIOS11.H"

#if FALSE
/* Format NKC Mini-Disk 800K */
drvprm F800 =
{
	"NKC800",
	{
		40,		/* spt */
		4,		/* bsh */
		15,		/* blm */
		0,		/* exm */
		389,	/* dsm */
		255,	/* drm */
		240,	/* al0 */
		0,		/* al1 */
		64,		/* cks */
		4,		/* ofs */
		/* blocking/deblocking */
		3,		/* psh */
		7,		/* phm */
		0x20,	/* reg */
		/* pyhsical prms */
		1024,	/* phylen */
		5-1,	/* physec */
		80-1,	/* phytrk */
		2		/* physid */
	}
};

drvprm F144 =
{
	"1.44MB",
	{
		72,		/* spt */
		5,		/* bsh */
		31,		/* blm */
		1,		/* exm */
		354,	/* dsm */
		255,	/* drm */
		192,	/* al0 */
		0,		/* al1 */
		64,		/* cks */
		2,		/* ofs */
		/* blocking/deblocking */
		3,		/* psh */
		7,		/* phm */
		0x00,	/* reg */
		/* pyhsical prms */
		1024,	/* phylen */
		9-1,	/* physec */
		80-1,	/* phytrk */
		2		/* physid */
	}
};

drvprm F140 =
{
	"1.40MB",
	{
		64,		/* spt */
		5,		/* bsh */
		31,		/* blm */
		1,		/* exm */
		315,	/* dsm */
		255,	/* drm */
		192,	/* al0 */
		0,		/* al1 */
		64,		/* cks */
		2,		/* ofs */
		/* blocking/deblocking */
		3,		/* psh */
		7,		/* phm */
		0x00,	/* reg */
		/* pyhsical prms */
		1024,	/* phylen */
		8-1,	/* physec */
		80-1,	/* phytrk */
		2		/* physid */
	}
};

drvprm F120 =
{
	"1.2MB",
	{
		60,		/* spt */
		5,		/* bsh */
		31,		/* blm */
		1,		/* exm */
		295,	/* dsm */
		255,	/* drm */
		192,	/* al0 */
		0,		/* al1 */
		64,		/* cks */
		2,		/* ofs */
		/* blocking/deblocking */
		2,		/* psh */
		3,		/* phm */
		0x00,	/* reg */
		/* pyhsical prms */
		512,	/* phylen */
		15-1,	/* physec */
		80-1,	/* phytrk */
		2		/* physid */
	}
};

/*
#define FMTCNT 4
drvprm *fmtlist[] = {F800, F144, F140, F120};
*/
#endif

/****************************************************************************/

#ASM
; get BIOS addr
;BIOSADR:
;	LD HL,(0001H)	; BIOS address
;	DEC HL
;	DEC HL
;	DEC HL
;	RET
	
; get phy. parameters
GETPHY:
	LD HL,(0001)		; BIOS addr start + 3
	LD DE,0036H-3
	ADD HL,DE
	JP (HL)
#ENDASM

/****************************************************************************/

/*
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
*/

/****************************************************************************/
/* Get current drive  */

int GetDrv()
{
#ASM
	LD A,(0004h)
	LD L,A
	LD H,0
#ENDASM
}	

/****************************************************************************/
/* Get BIOS version */

int GetVer()
{
#ASM
	LD HL,(0001H)		; BIOS addr start + 3
	LD DE,0033H-3		; BIOS function GETVER
	ADD HL,DE
	LD DE,getver1
	PUSH DE				; push RET addr
	JP (HL)
getver1:
	LD L,C				; BIOS version in C
	LD H,0
#ENDASM	
}

/****************************************************************************/
/* BIOS select drive for read/write */

char *SelDrv(drive)
	int drive;
{
#ASM
	POP BC
	POP HL
	PUSH HL
	PUSH BC
	LD C,L
	LD HL,(0001H)		; BIOS addr start + 3
	LD DE,001BH-3		; function SELDSK
	ADD HL,DE
	;LD DE,seldrv1
	;PUSH DE
	JP (HL)
;seldrv1:
#ENDASM	
}

/****************************************************************************/
/* get phys. bytes/sector for selected drive */

int GetByts()
{
#ASM
	CALL GETPHY
	RET
#ENDASM	
}

/****************************************************************************/
/* get phys. sectors/track for selected drive */

int GetSecs()
{
#ASM
	CALL GETPHY
	LD L,D
	LD H,0
	INC HL
	RET
#ENDASM	
}

/****************************************************************************/
/* get phys. sectors/track for selected drive */

int GetTrks()
{
#ASM
	CALL GETPHY
	LD L,C
	LD H,0
	INC HL
	RET
#ENDASM	
}

/****************************************************************************/
/* get phys. sides for selected drive */

int GetSids()
{
#ASM
	CALL GETPHY
	LD L,E
	LD H,0
	RET
#ENDASM	
}

/****************************************************************************/

main(argc, argv)
	int argc;
	char *argv[];
{
	BOOL error;
	int p, drive, curdrv, fmtidx;
	drvprm *fmt;
	drvtab *tab;
	unsigned *dpe;
	char *dpb, *tabptr;
	
	int ver, d;
	int phybyts, physecs, phytrks, physids;
	long size;
	
	printf("CHGDRV *dg* 260926-03, set NBIOS drive data\r\n");

	/* save current drive */
	curdrv = GetDrv();

	error = FALSE;
	drive = -1;
	fmt = NULL;
	
	if (argc < 2)
		error = TRUE;
	
	if (!error && strlen(argv[1]) == 2 && argv[1][1] == ':')
	{
		drive = toupper(argv[1][0]) - 'A';
		if (drive < 0 || drive > DRVCNT)
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
			else
				error = TRUE;
		}
	}
	
	if (error || drive == -1 || fmt == NULL)
	{
		printf("usage: CHGDRV <d>: -F<f>\r\n");
		printf("  d = Drive A..D\r\n");
		printf("  f = Format\r\n"); 		
		/* list all formats */
		for (p = 0; p < FMTCNT; p++)
		{
			tab = fmtlist[p]->tab;
			printf("   %d = %s %dK (%d/%d/%d/%d)\r\n", p+1, fmtlist[p]->name, fmtlist[p]->size,
				tab->phylen, tab->physec + 1, tab->phytrk + 1, tab->physid);
		}
		EXIT();
	}

	/* show NBIOS version */
	ver = GetVer();
	printf("\r\nNBIOS version %d.%d\r\n", ver / 10, ver % 10);
	
	/* show selected drive and format */
	tab = fmt->tab;
	printf("Drive=%c Fmt=%s %dK (%d/%d/%d/%d)\r\n", 'A'+drive, fmt->name, fmt->size,
		tab->phylen, tab->physec + 1, tab->phytrk + 1, tab->physid);

	/* pointer into table ot format parameters */
	tabptr = fmt->tab;

	/* pointer into BIOS drive parameters */
	dpe = SelDrv(drive);
	dpb = *(dpe + 5);
	printf("DPE=%04X DPB=%04X size=%d\r\n", dpe, dpb, sizeof(drvtab));
	
	/* patch drive parametes */
	for (p = 0; p < sizeof(drvtab); p++)
	{
		*(dpb + p) = *(tabptr + p);
	}

	/* show new BIOS drive parameters */
	printf("\r\ndrv by/se sects trks sids size  addr\r\n", ver);
	for (d = 0; d < DRVCNT; d++)
	{
		dpe = SelDrv(d);
		dpb = *(dpe + 5);
		phybyts = GetByts();
		physecs = GetSecs();
		phytrks = GetTrks();
		physids = GetSids();
		size = (long)phybyts * physecs * phytrks * physids;
		printf(" %c   %4d  %3d   %3d  %1d  %4dK  %04X\r\n", 'A' + d, phybyts, physecs, phytrks, physids, (int)(size / 1024), dpb);
	}

	/* restore current drive and exit */
	SelDrv(curdrv);
	EXIT();
}

/****************************************************************************/
