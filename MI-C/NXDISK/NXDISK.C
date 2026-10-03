end(
/****************************************************************************/
/*
	NXDISK.C  *dg*  10/2026 for MI-C compiler and CP/M
	
	Send / Receiove whole disk with Xmodem

	Uses BIOS conout/conin/const for fast xmodem transfer
	
*/
/****************************************************************************/
/* NXDISK version history

 02.10.2026 *dg* First version, working with 1,2 KB/s over 19200 baud
 03.10.2026 *dg* First version
 
 */

#include "stdio.h"
#include "conio.h"
#include "phydrv.h"

#define FALSE 0
#define TRUE 1
#define WORD unsigned int
#define BOOL int

/* serial port constants */
#ASM
FALSE	EQU 0
TRUE	EQU NOT FALSE

; BIOS
BIOS:	DW 0
BCONST: DS 3	; JP BCONST
BCONIN:	DS 3	; JP CONIN
BCONOUT:DS 3	; JP BCONOUT

; Monitor
MFLOP	EQU		0F021H	; Monior FLOP routine

EXTERNAL	SAST
EXTERNAL	SAIN
EXTERNAL	SAOUT

#ENDASM

/* transfer related constants */
#define SOH	0x01
#define STX	0x02
#define EOT	0x04
#define ACK	0x06
#define NAK	0x15
#define CTRLC 0x03
#define LF 0x0A
#define CR 0x0D

char *secbuf;

/*FILE *fplog;*/

/****************************************************************************/
/* creates table for fast jumps to BIOS I/O routines */

unsigned SetBios()
{
#ASM
	LD A,0C3H		; JP
	LD (BCONST),A
	LD (BCONIN),A
	LD (BCONOUT),A

	LD HL,(0001H)	; BIOS address + 3
	
	PUSH HL
	LD DE,0006H - 3
	ADD HL,DE
	LD (BCONST+1),HL
	POP HL

	PUSH HL
	LD DE,0009H - 3
	ADD HL,DE
	LD (BCONIN+1),HL
	POP HL

	PUSH HL
	LD DE,000CH - 3
	ADD HL,DE
	LD (BCONOUT+1),HL
	POP HL

	LD (BIOS),HL
#ENDASM
}

/****************************************************************************/
/* send character via BIOS */

Send(ch)
	int ch;
{
#ASM	
	POP HL
	POP BC			; get char in C
	PUSH BC
	PUSH HL
	
SEND1:
	CALL BCONOUT	; output char in C
	;LD A,C
	;CALL SAOUT
	LD L,C
	LD H,0
	RET
#ENDASM
}

/****************************************************************************/
/* recv character via BIOS, with timeout */
/* t = timeout in seconds (approximately, at 6 Mhz) */

int Recv(t)
	int t;
{
#ASM
	POP DE
	POP BC
	PUSH BC
	PUSH DE
	LD B,C			; timeout value

RECV1:
	LD DE,180 SHL 8			; 6 Mhz	(low byte must be 0)

RECV2:
	CALL BCONST
	;CALL SAST
	OR A
	JP NZ,RECV3		; -> got char
	
	; wait for char
	DEC E			; counter low
	JP NZ,RECV2
	DEC D			; counter high
	JP NZ,RECV2
	DEC B			; timeout counter
	JP NZ,RECV1

	; timeout, set carry
RECVTO:
	LD HL,0FFFFH	; -1 = timeout
	RET

RECV3:
	;CALL BCONIN		; char -> A
	CALL SAIN
	LD L,A			; return char in HL
	LD H,0
	RET
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
/* Monitor: read pyhsical sector */
/*
; HL = Buffer address
; E = Physical sector (1..n)
; D = Track (0..n-1)
; C = Drive-code + Density + Side (REG4-Latch)
; return 0 = no error
*/

BOOL Flop(cmd, drvcod, track, sector, addr)
	int cmd, drvcod, track, sector;
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
	CALL MFLOP	; Monitor FLOP routine
	LD L,A		; A=0: no error
	LD H,0
#ENDASM
}

/****************************************************************************/
/* drive 0..n-1 */
/* track 0..n-1 */
/* side 0..n.1 */
/* sector 0..n-1 */
/* FLO register */
/* addr = buffer address */

int ReadSector(drive, track, side, sector, reg, addr)
	int drive, track, side, sector, reg;
	unsigned addr;
{
	/*int drvcod = (1 << drive) | 0x20;*/
	int drvcod = (1 << drive) | reg;
	if (side == 1) drvcod |= 0x80;
	 /* read sector by Monitor */
	return Flop(1, drvcod, track, sector + 1, addr) == 0;
}

/****************************************************************************/
/* drive 0..n-1 */
/* track 0..n-1 */
/* side 0..n.1 */
/* sector 0..n-1 */
/* FLO register */
/* addr = buffer address */

int WriteSector(drive, track, side, sector, reg, addr)
	int drive, track, side, sector, reg;
	unsigned addr;
{
	/*cprintf("\r\nWR %d %d %d %d %u\t\n", drive, track, side, sector, addr);*/
	
	/*int drvcod = (1 << drive) | 0x20;*/
	int drvcod = (1 << drive) | reg;
	if (side == 1) drvcod |= 0x80;
	 /* write sector by Monitor */
	return Flop(2, drvcod, track, sector + 1, addr) == 0;
}

/****************************************************************************/
/* returns Ctrl-C if Ctrl-C pressed */ 

int Purge()
{
	int ch;
	
	while(TRUE)
	{
		ch = Recv(1);
		if (ch == -1) return 0;
		if (ch == CTRLC) return ch;
	}
}

/****************************************************************************/

int SendX(buf, blkno)
	char *buf;
	int blkno;
{
	int ch, i, chk;
	BOOL rept;
	int blklen;
	
	blklen = 128;

	/*fprintf(fplog, "SendX %04X %d %d\r\n", buf, blklen, blkno);*/
	
	rept = FALSE;
	while(TRUE)
	{
		/*fprintf(fplog, "cnt=%d\r\n", cnt);*/
		if (!rept) blkno++;

		/*fprintf(fplog, "blkno=%d\r\n", blkno);*/
		
		if (blklen == 128)
			Send(SOH);
		else
			Send(STX);
		
		Send(blkno);
		Send(blkno ^ 0xFF);
		
		chk = 0;
		for (i = 0; i < blklen; i++)
		{
			ch = buf[i];
			Send(ch);
			chk = (chk + ch) & 0xFF;
		}
		
		Send(chk);
		
		rept = FALSE;
		
		ch = Recv(4);
		/*fprintf(fplog, "resp=%02X\r\n", ch);*/
		if (ch == ACK) break; /* naechster Block */
		if (ch == CTRLC) return -1;
		
		/* repeat block */
		rept = TRUE;
	}
	/*fprintf(fplog, "ACK return blkno=%d\r\n", blkno);*/
	return blkno;
}

/****************************************************************************/

BOOL SendDisk(prm)
	drvprm *prm;
{
	int ch;
	int trk, sid, sec, blk;
	int blkno, blklen;
	BOOL success, abort;

	ch = Purge();
	if (ch == CTRLC) return FALSE;

	cprintf("\r\npurge = %02X\r\n", ch);

	/* wait for transfer start (NAK)*/
	while(TRUE)
	{
		ch = Recv(1);
		if (ch == CTRLC) return FALSE;
		if (ch == NAK) break;
	}

	cprintf("nak = %02X\r\n", ch);

	/*fprintf(fplog, "NAK recv\r\n");*/

	abort = FALSE;
	blklen = 128;
	blkno = 0;
	for (trk = 0; trk < prm->trks; trk++)
	{
		for (sid = 0; sid < prm->sids; sid++)
		{
			for (sec = 0; sec < prm->secs; sec++)
			{
				/*fprintf(fplog, "read sector %d %d %d %04X\r\n", trk, sid, sec, secbuf);*/
				success = ReadSector(prm->drive, trk, sid, sec, prm->floreg, secbuf);
				if (!success)
				{
					/* hier abbruch senden */
					Send(EOT);
					cputs("ReadSector error\r\n");
					goto abort;
				}
				for (blk = 0; blk < prm->byts / 128; blk++)
				{
					/*fprintf(fplog, "sendx %d %04X\r\n", blk, secbuf + blk * 128);*/
					blkno = SendX(secbuf + blk * 128, blkno);
					if (blkno == -1)
					{
						cputs("SendX error/abort\r\n");
						Send(EOT);
						return FALSE; /* ctrl-c received */
					}
				}
			}
		}
	}

abort:
	Send(EOT);
	ch = Recv(3);
	/*fprintf(fplog, "recv %02X\r\n", ch);*/
	if (ch != ACK)
	{	/* no ACK on EOT... */
		return FALSE;
	}
	
	cprintf("\r\n%ld bytes send\r\n", blkno * 128L);

	return TRUE;
}

/****************************************************************************/

int RecvX(buf, prvblk)
	char *buf;
	int prvblk;
{
	int ch;
	int i, blkno, blknoc, blklen;
	int chksum, chk;
	BOOL restrt, err;
	
	restrt = TRUE;
	err = FALSE;
	while (TRUE)
	{
		if (restrt)
		{
			ch = Recv(3);	/* recv with 3s timeout */
			if (ch == -1) err = TRUE;
			restrt = FALSE;
		}
		
		if (err)
		{
			/* timeout, wait until sender done */
			while(TRUE)
			{
				ch = Recv(1);
				if (ch == -1) break;;
			}
			Send(NAK);
			err = FALSE;
			restrt = TRUE;
			continue;
		}

		if (ch == CTRLC)
		{
			Send(ACK);
			/*fprintf(fplog, "CTRL-C\r\n");*/
			return -1;
		}

		if (ch == EOT)
		{
			Send(ACK);
			/*fprintf(fplog, "EOT\r\n");*/
			break;
		}

		if (ch == SOH)
			blklen = 128;
		else if (ch == STX)
			blklen = 1024;
		else
		{
			err = TRUE;
			continue;
		}

		/* recv block header */

		blkno = Recv(1);
		blknoc = Recv(1);
		if (blkno != (blknoc ^ 0xFF))
		{
			err = TRUE;
			continue;
		}
		
		/* recv block */
		
		chk = 0;
		for (i = 0; i < blklen; i++)
		{
			ch = Recv(1);
			buf[i] = ch;
			chk = (chk + ch) & 0xFF;
		}
		
		/* chksum */
		chksum = Recv(1);

		/*fprintf(fplog,"blkno=%02X %02X\r\n",blkno, blknoc);*/
		
		/*
		for (i = 0; i< 128; i++)
		{
			fprintf(fplog, "%3d %02X\r\n", i, secbuf[i]);
		}
		*/
		/*fprintf(fplog, "chk %02X %02X\r\n", chksum, chk);*/
		
		if (chksum != chk)
		{
			err = TRUE;
			continue;
		}
		
		break;
	}
	
	return blkno;
}

/****************************************************************************/

BOOL RecvDisk(prm)
	drvprm *prm;
{
	int ch;
	int trk, sid, sec, blk, blks;
	int prvblk, blkcnt;
	BOOL success, restrt;

	blks = prm->byts / 128;

	/* start transfer */
	Send(NAK);

	/*blklen = 128;*/
	prvblk = 0;
	blkcnt = 0;
	for (trk = 0; trk < prm->trks; trk++)
	{
		for (sid = 0; sid < prm->sids; sid++)
		{
			for (sec = 0; sec < prm->secs; sec++)
			{
				for (blk = 0; blk < blks; blk++)
				{
					/*fprintf(fplog, "recvx t=%d s=%d sec=%d b=%d %04X\r\n", trk, sid, sec, blk, secbuf + blk * 128);*/
					prvblk = RecvX(secbuf + blk * 128, prvblk);
					if (prvblk == -1)
					{
						/*fprintf(fplog, "prvblk==-1, abbruch\r\n");*/
						cputs("RecvX error/abort\r\n");
						return FALSE; /* ctrl-c received */
					}
					blkcnt++;
					if (blk < blks - 1)
					{
						/*fprintf(fplog, "blk send ack\r\n");*/
						Send(ACK);
					}
				}
				/*fprintf(fplog, "write sector %d %d %d %04X\r\n", trk, sid, sec, secbuf);*/
				success = WriteSector(prm->drive, trk, sid, sec, prm->floreg, secbuf);
				if (!success)
				{
					/* hier abbruch senden */
					Send(CTRLC);
					cputs("WriteSector error\r\n");
					return FALSE;
				}
				/*fprintf(fplog, "sec send ack\r\n");*/
				Send(ACK);
			}
		}
	}

	/* wait for EOT */
	while(TRUE)
	{
		ch = Recv(1);
		if (ch == CTRLC) return FALSE;
		if (ch == NAK) break;
		if (ch == EOT)
		{
			Send(ACK);
			break;
		}
	}
	/*fprintf(fplog, "recv %02X\r\n", ch);*/
	
	cprintf("\r\n%ld bytes received\r\n", blkcnt * 128L);

	return TRUE;
}

/****************************************************************************/

#define DIRNON 0
#define DIRSND 1
#define DIRRCV 2

main(argc, argv)
	int argc;
	char *argv[];
{
	int ch;
	int drive, p;
	int dir, tracks;
	BOOL error;
	drvprm prm;

	SetBios();
	SA_Init();

	cputs("NXDISK *dg* v1.0 281002-06\r\n");

	error = FALSE;
	if (argc < 2)
		error = TRUE;

	dir = DIRRCV;
	tracks = -1;
	if (!error)
	{
		for (p = 1; p < argc; p++)
		{
			if (strlen(argv[p]) == 2 && argv[p][1] == ':')
			{
				drive = toupper(argv[p][0]) - 'A';
				if (drive < 0 || drive > 15) error = TRUE;
				continue;
			}

			switch(toupper(argv[p][0]))
			{
				case 'R':
					dir = DIRRCV;
					break;
				case 'S':
					dir = DIRSND;
					break;
				case '-':
				case '/':
					if (toupper(argv[p][1]) == 'T' && strlen(argv[p]) > 2)
					{
						tracks = atoi(argv[p] + 2);
						if (tracks > 255) error = TRUE;
					}
					break;
			}
			if (error) break;
		}
	}
	if (dir == DIRNON) error = TRUE;
	
	if (error)
	{
		cputs("\r\nusage: NXDISK s/r <d>: -t<n>\r\n");
		cputs("  s / r = send or receive\r\n");
		cputs("  <d>:  = disk drive\r\n");
		cputs("  -t<n> = tracks\r\n");
		return;
	}

	/* get disk parameters */
	GetPrm(drive, &prm);
	
	/* overwrite disk parameters */
	if (tracks != -1) prm.trks = tracks;

	/* allocate memory for sector buffer */
	secbuf = CALLOC(prm.byts, 1);
	if (secbuf == NULL)
	{
		cprintf("Error: not enough memory (%d)\r\n", prm.byts);
		return;
	}

	if (dir == DIRSND)
	{
		cputs("Send ");
	}
	else
	{
		cputs("Recv ");
	}
	
	cprintf("disk = %c: (%d/%d/%d/%d/%02X/%dK)\r\n", drive+'A',
		prm.byts, prm.secs, prm.trks, prm.sids, prm.floreg, prm.size);

	cprintf("\r\nInsert disk in drive %c and press ENTER\r\n", drive + 'A');
	ch = WaitCr();
	if (ch == CTRLC) EXIT();

	/*
	fplog = fopen("nxdisk.log", "wa");
	if (fplog==NULL) return;
	*/

	if (dir == DIRSND)
	{
		cputs("Receive disk image now using Xmodem...");
		SendDisk(&prm);
	}
	else
	{
		cputs("Send disk image now using Xmodem...");
		RecvDisk(&prm);
	}
	

	/*fclose(fplog);*/

	cputs("\r\nInsert SYSTEM disk\r\n");
	WaitCr();

	EXIT();
}

/****************************************************************************/
