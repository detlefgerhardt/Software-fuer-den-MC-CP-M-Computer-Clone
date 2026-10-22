/****************************************************************************/
/*
	COPYCF.C  *dg*  10/2026 for MI-C compiler and MC CP/M clone
	
	Copy a CF cards
*/
/****************************************************************************/
/* DISKCOPY version history

 16.10.2026 *dg* First version
 
 */

#include "stdio.h"
#include "phydrv.h"

#define FALSE 0
#define TRUE 1
#define WORD unsigned int
#define BOOL int

#define CTRLC 3
#define CR 13

#define DRVCNT 4 /* number of drives */
#define SECSIZ 256
#define SECCNT 128
#define TRKCNT 256

char *buffer;
char *vbuffer;

/****************************************************************************/
/* BDOS init disk system */

InitDisks()
{
	BDOS(0, 0x0D);
}

/****************************************************************************/
/* dump memory from address p, count bytes
   use dspadr as the displayed address
*/

dump(p, count, dspadr)
	char *p;
	int count;
	unsigned dspadr;
{
	int cnt,col,ch;
	char s[5];
	int cols = 16;

	if (dspadr == 0XFFFF) dspadr = p;

	cnt = 0;
	while(cnt < count)
	{
		utohex(dspadr, s);
		cputs(s);
		for (col = 0; col < cols; col++)
		{
			putch(' ');
			if (cnt + col < count)
			{
				ctohex(*(p+col), s);
				cputs(s);
			}
			else
				cputs("  ");
			if (col == 7)
				putch(' ');
		}
		putch(' ');
		for (col = 0; col < cols; col++)
		{
			if (cnt + col >= count) break;
			ch = *(p+col);
			if (ch >= 0x20 && ch <= 0x7E)
				putch(ch);
			else
				putch('.');
			if (col == 7)
				putch(' ');
		}
		cputs("\r\n");
		cnt += cols;
		p += cols;
		dspadr += cols;
	}
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

BOOL cmpprm(src, dst)
	phyprm *src, *dst;
{
	if (src->byts != dst->byts) return FALSE;
	if (src->secs != dst->secs) return FALSE;
	if (src->trks != dst->trks) return FALSE;
	if (src->sids != dst->sids) return FALSE;
	if (src->floreg != dst->floreg) return FALSE;
	return TRUE;
}

/****************************************************************************/

BOOL Copy(srcadr, srcdrv, dstadr, dstdrv, trkcnt, verify)
	int srcadr, srcdrv;
	int dstadr, dstdrv;
	int trkcnt;
	BOOL verify;
{
	int sec, trk, cnt;
	
	for (trk = 0; trk < trkcnt; trk++)
	{
		cprintf("T=%3d\r", trk);
		for (sec = 0; sec < SECCNT; sec++)
		{
			if (kbhit())
			{
				if (getch() == CTRLC)
				{
					cputs("\r\n\ncopy aborted\r\n");
					return FALSE;
				}
			}
			if (srcdrv != -1)
			{
				Read(buffer, srcadr, srcdrv, trk, sec);
			}
			if (dstdrv != -1)
			{
				Write(buffer, dstadr, dstdrv, trk, sec);
			}
			if (verify)
			{
				Read(vbuffer, dstadr, dstdrv, trk, sec);
				if (memcmp(vbuffer, buffer, SECSIZ) != NULL)
				{
					cprintf("\r\n\nverify error at sector %d / track %d\r\n", sec, trk);
					cputs("copy aborted\r\n");
					return FALSE;
				}
			}
		}
	}

	cputs("\r\n\ncopy finished\r\n");
	return TRUE;
}

/****************************************************************************/

#if FALSE
int TestSize()
{
	int i, ch;
	long lba;
	int save1, save2;
	
	buffer = calloc(SECSIZ, 1);
	if (buffer == NULL)
	{
		cprintf("Not enough memory for sector buffer (%u)\r\n", SECSIZ);
		return FALSE;
	}
	vbuffer = calloc(SECSIZ, 1);
	if (vbuffer == NULL)
	{
		cprintf("Not enough memory for sector buffer 2 (%u)\r\n", SECSIZ);
		return FALSE;
	}

	for (i = 0; i < 64; i++)
	{
		if (kbhit())
		{
			if (getch() == CTRLC) break;
		}
		lba = i * 0x4000L;
		cprintf("lba = %06lX %ldMB\r\n", lba, lba * 256L / (1024L * 1024L));
		RD_LBA(buffer, 0x18, lba);
		cputs("read ok\r\n");
		save1 = buffer[0];
		save2 = buffer[1];
		buffer[0] = i;
		buffer[1] = i ^ 0xFF;
		WR_LBA(buffer, 0x18, lba);
		cputs("write ok\r\n");
		RD_LBA(buffer, 0x18, lba);
		if (buffer[0] != i || buffer[1] != (i ^ 0xFF))
		{
			cprintf("error at %d (%02X %02X)\r\n", i, buffer[0], buffer[1]);
			break;
		}
		buffer[0] = save1;
		buffer[1] = save2;
		WR_LBA(buffer, 0x18, lba);
	}

	/*
	dump(buffer, 256, 0x1800);
	cputs("\r\n");
	RD_LBA(buffer, 0x18, 0x0001L);
	dump(buffer, 256, 0x1800);
	cputs("\r\n");
	*/
}
#endif

/****************************************************************************/

int NotBsy(flag)
	int flag;
{
	int d, i;
	int addr = 0x18;
	
	for (i = 0; i< 20; i++)
	{
		d = inp(addr + 7);	/* status */
		if ((d & 0x80) == 0)
		{
			cprintf("BUSY %d=%d (%02X)\r\n", flag, i, d);
			return TRUE;
		}
	}
	cprintf("BUSY %d=timeout (%02X)\r\n", flag, d);
	return FALSE;
}

/****************************************************************************/

int Rdy(flag)
	int flag;
{
	int d, i;
	int addr = 0x18;
	
	for (i = 0; i< 100; i++)
	{
		d = inp(addr + 7);	/* status */
		if ((d & 0x40) != 0)
		{
			cprintf("RDY %d=%d (%02X)\r\n", flag, i, d);
			return TRUE;
		}
	}
	cprintf("RDY %d=timeout (%02X)\r\n", flag, d);
	return FALSE;
}

/****************************************************************************/

int Drq(flag)
	int flag;
{
	int d, i;
	int addr = 0x18;
	
	for (i = 0; i< 1000; i++)
	{
		d = inp(addr + 7);	/* status */
		if ((d & 0x08) != 0)
		{
			if (flag != 0)
			{
				cprintf("DRQ %d=%d (%02X)\r\n", flag, i, d);
			}
			return TRUE;
		}
	}
	cprintf("DRQ %d=timeout (%02X)\r\n", flag, d);
	return FALSE;
}

/****************************************************************************/

TestCard()
{
	int d, i;
	int addr = 0x18;
	
	cputs("TestCard\r\n");
	
	buffer = calloc(512, 1);
	if (buffer == NULL)
	{
		cprintf("Not enough memory for sector buffer (%u)\r\n", 512);
		return FALSE;
	}

	if (!NotBsy(1)) return;
	if (!Rdy(1)) return;

	
#if FALSE
	outp(addr + 7, 0x04); /* reset command */

	if (!NotBsy(2)) return;
	if (!Rdy(2)) return;
#endif

	outp(addr + 6, 0xE0); /* lba-mode, drive, head */
	/*outp(addr + 1, 0x01);*/ /* 8 bit mode to feature register */
	outp(addr + 1, 0x81); /* 16 bit mode to feature register */
	outp(addr + 7, 0xEF); /* "set feature" command to command register */

	if (!NotBsy(3)) return;
	/*if (!Rdy(3)) return;*/

	outp(addr + 2, 1); /* sector count */
	
	outp(addr + 3, 0); /* lba7 */
	outp(addr + 4, 0); /* lba15 */
	outp(addr + 5, 0); /* lba23 */
	/*outp(addr + 6, 0xE0);*/ /* lba-mode, drive, head */

	outp(addr + 7, 0x20);	/* read sector command */

	if (!Drq(4)) return;

	if (!NotBsy(4)) return;

	inp(addr + 7); /* clear INTRQ */
	
	for (i = 0; i < 512; i++)
	{
		buffer[i] = inp(addr);
		if ((inp(addr + 7) & 08) == 0)
		{
			cprintf("DRQ low at %d\r\n", i);
			break;
		}
	}
	
	dump(buffer, 512, 0x0000);
	
#if FALSE	
	outp(addr + 7, 0x04); /* reset command */
#endif	
}

/****************************************************************************/



/****************************************************************************/

main(argc, argv)
	int argc;
	char *argv[];
{
	phyprm srcprm, dstprm;
	int p, ch;
	int srcadr, dstadr;
	int srcdrv, dstdrv;
	int tracks;
	BOOL verify;
	BOOL error;

	cputs("\r\nCOPYCF V1.0 for MC CP/M *dg* 261022-03\r\n\n");

	tracks = TRKCNT;
	verify = FALSE;
	error = FALSE;

#if TRUE	
	if (argc < 4)
		error = TRUE;
	else
	{
		srcadr = hextou(argv[1]);
		srcdrv = atoi(argv[2]);
		if (srcdrv < -1 || srcdrv > DRVCNT) error = TRUE;
		dstadr = hextou(argv[3]);
		dstdrv = atoi(argv[4]);
		if (dstdrv < 0 || dstdrv > DRVCNT) error = TRUE;
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
			if (toupper(argv[p][1]) == 'T' && strlen(argv[p]) >= 3)
			{
				tracks = atoi(argv[p]);
				if (tracks < 1 || tracks > 256)
				{
					error = TRUE;
					break;
				}
			}
		}
	}
#else
	srcadr = 0x30
	srcdrv = 0;
	dstadr = 0x18
	dstdrv = 0;
	tracks = 256;
	error = FALSE;
	verify = FALSE;
#endif
	

	if (error)
	{
		cprintf("Usage: copycf src-adr src-drv dst-adr dst-drv -v\r\n");
		cputs  ("  src-adr,dst-adr = controllar address as 2 char hex value (e.g. 30)\r\n");
		cputs  ("  src-drv,dst-drv = partition number 0..3\r\n");
		cputs  ("  -v = optional verify\r\n");
		cputs  ("  -t = optional number of tracks to copy (1..256)\r\n");
		cputs  ("  -t = optional number of tracks to copy (1..256)\r\n");
		cputs  ("  src-drv = -1: no src-drv, clear dst-drv with E5h\r\n");
		cputs  ("  dst-drv = -1: no dst-drv, test-read src-drv\r\n");
		return;
	}

	/*
	GetPrm(src, &srcprm);
	GetPrm(dest, &dstprm);

	cprintf("src %c: (%d/%d/%d/%d/%02X/%dK)\r\n", src+'A', srcprm.byts, srcprm.secs, srcprm.trks, srcprm.sids, srcprm.floreg, srcprm.size);
	cprintf("dst %c: (%d/%d/%d/%d/%02X/%dK)\r\n", dest+'A', dstprm.byts, dstprm.secs, dstprm.trks, dstprm.sids, dstprm.floreg, dstprm.size);
	*/

	buffer = calloc(SECSIZ, 1);
	if (buffer == NULL)
	{
		cprintf("Not enough memory for sector buffer (%u)\r\n", SECSIZ);
		return FALSE;
	}
	
	if (dstdrv == -1) verify = FALSE;
	if (verify)
	{
		vbuffer = calloc(SECSIZ, 1);
		if (buffer == NULL)
		{
			cprintf("Not enough memory for verify buffer (%u)\r\n", SECSIZ);
			return FALSE;
		}
	}

	cprintf("Copy from [%02Xh #%d] to [%02Xh #%d]\r\n", srcadr, srcdrv, dstadr, dstdrv);
	if (verify)
	{
		cputs("verify = ON\r\n");
	}

	cputs("Press ENTER to start\r\n\n");
	ch = WaitCr();
	if (ch == CTRLC) return;

	if (srcdrv == -1)
	{
		memset(buffer, 0xE5, SECSIZ);
	}

	Copy(srcadr, srcdrv, dstadr, dstdrv, tracks, verify);

	/*InitDisks();
	return; */

	EXIT();
}

/****************************************************************************/
