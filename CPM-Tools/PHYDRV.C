/****************************************************************************/
/*
	PHYDRV.C  *dg*  09/2026 for MI-C compiler and MC CP/M clone
	
	Shows physical drive parameters. Needs NBIOS 1.0 with new functions
	GETVER and GETPHY
*/
/****************************************************************************/
/* PHYDRV version history

 022.09.2026 *dg* First version
 
 */

#include "stdio.h"

#define FALSE 0
#define TRUE 1
#define WORD unsigned int
#define BOOL int

#define CTRLC 3
#define CR 13

#define DRVCNT 8 /* number of drives */

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
	int ver, d;
	int phybyts, physecs, phytrks, physids;
	long size;
	
	printf("PHYDRV *dg* 260922-01, physical drive data for NBIOS\r\n");
	
	ver = GetVer();
	printf("NBIOS version %d.%d\r\n\n", ver / 10, ver % 10);
	
	printf("drv by/se sects trks sids size\r\n", ver);
	for (d = 0; d < DRVCNT; d++)
	{
		SelDrv(d);
		phybyts = GetByts();
		physecs = GetSecs();
		phytrks = GetTrks();
		physids = GetSids();
		size = (long)phybyts * physecs * phytrks * physids;
		printf(" %d   %4d  %3d   %3d  %1d   %dK\r\n", d, phybyts, physecs, phytrks, physids, (int)(size / 1024));
	}
}

/****************************************************************************/
