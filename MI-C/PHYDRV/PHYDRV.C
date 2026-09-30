/****************************************************************************/
/*
	PHYDRV.C  *dg*  09/2026 for MI-C compiler and MC CP/M clone
	
	Shows physical drive parameters. Needs NBIOS at least 1.0 with new
	functions GETVER and GETPHY
*/
/****************************************************************************/
/* PHYDRV version history

 22.09.2026 *dg* First version
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
	
	cprintf("drv by/se sects trks sids size  addr\r\n");
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
	int ver;
	
	cprintf("PHYDRV *dg* 260930-01, physical drive data for NBIOS\r\n");
	
	ver = GetVer();
	cprintf("NBIOS version %d.%d\r\n\n", ver / 10, ver % 10);

	ShowDrives(DRVCNT);
}

/****************************************************************************/
