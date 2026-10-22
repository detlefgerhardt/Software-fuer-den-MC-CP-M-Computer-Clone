/****************************************************************************/
/*
	TESTCF.C  *dg*  10/2026 for MI-C compiler and MC CP/M clone
	
	Test CF cards
*/
/****************************************************************************/
/* DISKCOPY version history

 22.10.2026 *dg* First version
 
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

#if FALSE

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
			Read(buffer, srcadr, srcdrv, trk, sec);
			Write(buffer, dstadr, dstdrv, trk, sec);
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

#endif

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
			cprintf("BUSY F%d C%d (%02Xh)\r\n", flag, i, d);
			return TRUE;
		}
	}
	cprintf("BUSY F%d timeout (%02Xh)\r\n", flag, d);
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
			cprintf("RDY F%d C%d (%02Xh)\r\n", flag, i, d);
			return TRUE;
		}
	}
	cprintf("RDY F%d timeout (%02Xh)\r\n", flag, d);
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
				cprintf("DRQ F%d C%d (%02Xh)\r\n", flag, i, d);
			}
			return TRUE;
		}
	}
	cprintf("DRQ F%d timeout (%02Xh)\r\n", flag, d);
	return FALSE;
}

/****************************************************************************/

BOOL Init(addr)
	int addr;
{
	int d, i;
	int addr = 0x18;
	
	cputs("Init\r\n");
	
	if (!NotBsy(1)) return FALSE;
	if (!Rdy(2)) return FALSE;
	
#if FALSE
	outp(addr + 7, 0x04); /* reset command */

	if (!NotBsy(3)) return FALSE;
	if (!Rdy(4)) return FALSE;
#endif

	outp(addr + 6, 0xE0); /* lba-mode, drive, head */
	/*outp(addr + 1, 0x01);*/ /* 8 bit mode to feature register */
	outp(addr + 1, 0x81); /* 16 bit mode to feature register */
	outp(addr + 7, 0xEF); /* "set feature" command to command register */

	if (!NotBsy(5)) return FALSE;
	/*if (!Rdy(3)) return;*/

	outp(addr + 2, 1); /* sector count */
	
	outp(addr + 3, 0); /* lba7 */
	outp(addr + 4, 0); /* lba15 */
	outp(addr + 5, 0); /* lba23 */
	/*outp(addr + 6, 0xE0);*/ /* lba-mode, drive, head */
	
	cputs("init ok\r\n");
}

/****************************************************************************/

TestCard()
{
	int i;
	int addr = 0x18;
	
	buffer = calloc(512, 1);
	if (buffer == NULL)
	{
		cprintf("Not enough memory for sector buffer (%u)\r\n", 512);
		return FALSE;
	}

	cputs("--- Read ---\r\n");

	Init(addr);

	outp(addr + 7, 0x20);	/* read sector command */
	if (!Drq(10)) return;
	if (!NotBsy(11)) return;
	inp(addr + 7); /* clear INTRQ */
	
	for (i = 0; i < 512; i++)
	{
		buffer[i] = inp(addr);
		if ((inp(addr + 7) & 08) == 0)
		{
			cprintf("DRQ low at %d\r\n (read)", i);
			break;
		}
	}
	dump(buffer, 512, 0x0000);

	/* --- write sector */

	cputs("--- Write ---\r\n");

	memset(buffer, 0xE5, 512);

	Init(addr);

	outp(addr + 7, 0x30);	/* read sector command */
	if (!Drq(20)) return;
	if (!NotBsy(21)) return;
	inp(addr + 7); /* clear INTRQ */

	for (i = 0; i < 512; i++)
	{
		outp(addr, buffer[i]);
		if ((inp(addr + 7) & 08) == 0)
		{
			cprintf("DRQ low at %d (write)\r\n", i);
			break;
		}
	}
	if (!NotBsy(22)) return;
	inp(addr + 7);	/* clear INTRQ */
	
#if FALSE	
	outp(addr + 7, 0x04); /* reset command */
#endif	
}

/****************************************************************************/

main(argc, argv)
	int argc;
	char *argv[];
{
	cputs("\r\nTESTCF V1.0 for MC CP/M *dg* 261022-01\r\n\n");

	/*TestSize();*/
	TestCard();
	return;

	EXIT();
}

/****************************************************************************/
