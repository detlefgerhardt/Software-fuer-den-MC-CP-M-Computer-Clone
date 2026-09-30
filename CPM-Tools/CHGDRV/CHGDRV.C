/****************************************************************************/
/*
	CHGDRV.C  *dg*  09/2026 for MI-C compiler and MC CP/M clone
	
	Changes drive parameters at runtime
*/
/****************************************************************************/
/* CHGDRV version history

 25.09.2026 *dg* First version
 30.09.2026 *dg* Use library functions
 
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

#include "NBIOS11.DH"

/****************************************************************************/

ShowFmts(fmtCnt, indent)
	int fmtCnt;
	char *indent;
{
	int f;
	drvtab *tab;
	
	for (f = 0; f < FMTCNT; f++)
	{
		tab = fmtlist[f]->tab;
		if (indent != NULL)
			cputs(indent);
		cprintf("%d = %s %dK (%d/%d/%d/%d)\r\n", f+1, fmtlist[f]->name, fmtlist[f]->size,
			tab->phylen, tab->physec + 1, tab->phytrk + 1, tab->physid);
	}
}	

/****************************************************************************/

ShowDrives(drvcnt)
	int drvcnt;
{
	int d, curdrv;
	int phybyts, physecs, phytrks, physids;
	long size;
	unsigned *dpe;
	char *dpb;

	/* save current drive */
	curdrv = GetDrv();
	
	cprintf("\r\ndrv by/se sects trks sids size  addr\r\n");
	for (d = 0; d < drvcnt; d++)
	{
		dpe = SelDrv(d);
		dpb = *(dpe + 5);
		phybyts = GetByt();
		physecs = GetSec();
		phytrks = GetTrk();
		physids = GetSid();
		size = (long)phybyts * physecs * phytrks * physids;
		cprintf(" %c   %4d  %3d   %3d  %1d  %4dK  %04X\r\n", 'A' + d, phybyts, physecs, phytrks, physids, (int)(size / 1024), dpb);
	}

	/* restore current drive and exit */
	SelDrv(curdrv);
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
	/*int phybyts, physecs, phytrks, physids;*/
	long size;
	
	cputs("CHGDRV *dg* 260930-01, set NBIOS drive data\r\n");

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
		cputs("usage: CHGDRV <d>: -F<f>\r\n");
		cputs("  d = Drive A..D\r\n");
		cputs("  f = Format\r\n"); 		
		/* list all formats */
		ShowFmts(FMTCNT, "   ");
		EXIT();
	}

	/* show NBIOS version */
	ver = GetVer();
	cprintf("\r\nNBIOS version %d.%d\r\n", ver / 10, ver % 10);
	
	/* show selected drive and format */
	tab = fmt->tab;
	cprintf("Drive=%c Fmt=%s %dK (%d/%d/%d/%d)\r\n", 'A'+drive, fmt->name, fmt->size,
		tab->phylen, tab->physec + 1, tab->phytrk + 1, tab->physid);

	/* pointer into table ot format parameters */
	tabptr = fmt->tab;

	/* save current drive */
	curdrv = GetDrv();

	/* pointer into BIOS drive parameters */
	dpe = SelDrv(drive);
	dpb = *(dpe + 5);
	cprintf("DPE=%04X DPB=%04X size=%d\r\n", dpe, dpb, sizeof(drvtab));
	
	/* patch drive parametes */
	for (p = 0; p < sizeof(drvtab); p++)
	{
		*(dpb + p) = *(tabptr + p);
	}

	/* restore current drive */
	SelDrv(curdrv);

	/* show new BIOS drive parameters */
	ShowDrives(DRVCNT);
	
	EXIT();
}

/****************************************************************************/
