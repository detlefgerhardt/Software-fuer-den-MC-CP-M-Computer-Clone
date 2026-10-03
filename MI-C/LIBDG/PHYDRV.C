/* PHYDRV.C *dg* 02.10.2026 */
/* function for NBIOS GETPHY funtion */

#include "PHYDRV.H"

/*
#define FALSE 0
#define TRUE 1
#define WORD unsigned int
#define BOOL int
*/

#ASM

BGETVER	EQU 0033h		; BIOS function GETVER
BGETPHY	EQU 0036h		; BIOS function GETPHY

; get phy. parameters
GETPHY:
	LD HL,(0001)		; BIOS addr start + 3
	LD DE,BGETPHY-3
	ADD HL,DE
	JP (HL)
#ENDASM

/****************************************************************************/
/* Get BIOS version */

int GetVer()
{
#ASM
	LD HL,(0001H)		; BIOS addr start + 3
	LD DE,BGETVER-3		; BIOS function GETVER
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
/* get phys. bytes/sector for selected drive */

int GetByt()
{
#ASM
	CALL GETPHY
	RET
#ENDASM	
}

/****************************************************************************/
/* get phys. sectors/track for selected drive */

int GetSec()
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

int GetTrk()
{
#ASM
	CALL GETPHY
	LD L,E
	LD H,0
	INC HL
	RET
#ENDASM	
}

/****************************************************************************/
/* get phys. sides for selected drive */

int GetSid()
{
#ASM
	CALL GETPHY
	LD L,B
	LD H,0
	RET
#ENDASM	
}

/****************************************************************************/
/* get phys. sides for selected drive */

int GetReg()
{
#ASM
	CALL GETPHY
	LD L,C
	LD H,0
	RET
#ENDASM	
}

/****************************************************************************/
/* get physical parameters of drive <drive>
   *prm is call by reference */

GetPrm(drive, prm)
	int drive;
	phyprm *prm;
{
	int curdrv;

	curdrv = GetDrv();
	SelDrv(drive);

	prm->drive = drive;
	prm->byts = GetByt();
	prm->secs = GetSec();
	prm->trks = GetTrk();
	prm->sids = GetSid();
	prm->floreg = GetReg();
	prm->side = 0; /* not used */
	prm->size = (int)((long)prm->byts * prm->secs * prm->trks * prm->sids / 1024);

	SelDrv(curdrv);
}

/****************************************************************************/
