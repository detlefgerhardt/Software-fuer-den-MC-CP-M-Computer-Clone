/****************************************************************************/
/*
	SHOWFMTS.C  *dg*  10/2026 for MI-C compiler and MC CP/M clone
	
	Shows physical drive parameters. Needs NBIOS at least 1.0 with new
	functions GETVER and GETPHY
*/
/****************************************************************************/
/* SHOWFMTS version history

 22.09.2026 *dg* First version
 30.09.2026 *dg* Use library functions
 05.10.2026 *dg* Renamed to SHOWFMTS
 
 */

#include "stdio.h"
#include "phydrv.h"

#define FALSE 0
#define TRUE 1
#define WORD unsigned int
#define BOOL int

#define CTRLC 3
#define CR 13

#define DRVCNT 8 /* number of drives */

/****************************************************************************/

ShowDrives(drvcnt)
	int drvcnt;
{
	int d, curdrv;
	phyprm prm;
	unsigned *dpe;
	char *dpb;

	/* save current drive */
	curdrv = GetDrv();
	
	cprintf("drv by/se sects trks sids reg  size  addr\r\n");
	for (d = 0; d < drvcnt; d++)
	{
		dpe = SelDrv(d);
		dpb = *(dpe + 5);
		GetPrm(d, &prm);
		cprintf(" %c   %4d  %3d   %3d  %1d    %02X %4dK  %04X\r\n", 'A' + d,
			prm.byts, prm.secs, prm.trks, prm.sids, prm.floreg, prm.size, dpb);
	}

	/* restore current drive and exit */
	SelDrv(curdrv);
}

/****************************************************************************/

main(argc, argv)
	int argc;
	char *argv[];
{
	int ver;
	
	cprintf("SHOWFMTS *dg* 261005-01, show physical drive formats for NBIOS\r\n");
	
	ver = GetVer();
	cprintf("NBIOS version %d.%d\r\n\n", ver / 10, ver % 10);

	ShowDrives(DRVCNT);
}

/****************************************************************************/
