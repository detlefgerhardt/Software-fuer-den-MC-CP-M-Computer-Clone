/****************************************************************************/
/*
	YMODEM.C  *dg*  10/2026 for MI-C compiler and CP/M
	
	getpc/putpc/Xmodem implementation
	
	Uses BIOS conout/conin/const for fast ymodem transfer
	
*/
/****************************************************************************/
/* YMODEM version history

 08.10.2026 *dg* First version from Xmodem
 
 */

#include "stdio.h"

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

#ENDASM

/* transfer related constants */
#define SOH	0x01
#define STX	0x02
#define EOT	0x04
#define ACK	0x06
#define NAK	0x15
#define NAKCRC 0x43
#define CTRLC 0x03
#define LF 0x0A
#define CR 0x0D

long crctab[256];

char blkbuf[1024 + 4];

extern sa_out();

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
/* send character via UART (not used) */

#if FALSE

Send(ch)
	int ch;
{
#ASM	
	POP BC
	POP HL
	PUSH HL
	PUSH BC
	
SEND1:
	; wait for send buffer empty
	IN A,(SIOACR)
	AND XMTMASK
	;CP XMTRDY
	JP Z,SEND1		; -> not empty

	LD A,L
	OUT (SIOADR),A
	LD H,0
	RET
#ENDASM
}

#ENDIF

/****************************************************************************/
/* recv character via UART (no used) */

#IF FALSE

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
	;LD DE,159 SHL 7	; 4 MHZ *)	
	LD DE,159 SHL 8		; 49 cycle loop, 6.272ms/wrap * 159 = 1 second

RECV2:
	IN A,(SIOACR)
	AND RCVMASK
	;CP RCVRDY
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
	IN A,(SIOADR)
	LD L,A			; return char in HL
	LD H,0
	RET
#ENDASM
}

#ENDIF

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
	;LD DE,159 SHL 7	; 4 MHZ *)	
	;LD DE,159 SHL 8		; 49 cycle loop, 6.272ms/wrap * 159 = 1 second (6 MHz)
	;LD DE,120 SHL 8		; 4 Mhz	(low byte must be 0)
	LD DE,180 SHL 8			; 6 Mhz	(low byte must be 0)

RECV2:
	CALL BCONST
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
	CALL BCONIN		; char -> A
	LD L,A			; return char in HL
	LD H,0
	RET
#ENDASM
}

/****************************************************************************/
/* selbstdefiniertes printf mit variabler parameterliste args */

xprintf(fmt, args)
	char *fmt;
	int *args;
{
	/* aufruf von print mit zeiger auf die parameterliste args */
	print(sa_out, fmt, &args);
}

/****************************************************************************/

crcinit()
{
	int i, j;
	long crc;

	for (i = 0; i < 256; ++i)
	{
		crc = (long)i * 256L;

		for (j = 0; j < 8; ++j)
		{
			if (crc & 32768L)
				crc = ((crc * 2L) & 65535L) ^ 4129L;
			else
				crc = (crc * 2L) & 65535L;
		}
		crctab[i] = crc;
	}
}

/****************************************************************************/
/*
 * Berechnet die CRC von n Bytes ab p.
 * Polynom:     0x1021 (x^16 + x^12 + x^5 + 1)
 * Startwert:   0x0000
 * Bytefolge:   hoechstwertiges CRC-Byte zuerst
 * *hip und *lop erhalten High- bzw. Low-Byte der CRC, jeweils 0..255.
 * Das '& 0377' macht die Routine unabhaengig davon, ob char signed ist.
 * erstellt mit OpenAI Codex 10/2026 *
 */

long crc16(ch, crc)
	int ch;
	long crc;
{
	int t;

	t = (int)((crc / 256L) ^ (ch & 0377));
	return ((crc * 256L) & 65535L) ^ crctab[t];
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

dump(sr, i)
	int sr, i;
{
	char s[3];
	
	sa_out(sr);
	sa_puts(ctohex(i, s));
	sa_out(' ');
}

/****************************************************************************/
/* receive one block */

int RecvX(init, buf, prvblk, len, flag)
	BOOL init;
	char *buf;
	int prvblk;
	int *len;
	int flag;
{
	int ch;
	int i, blkno, blklen;
	int start;
	int crc1, crc2;
	long crc;
	BOOL restrt, err;

	xprintf("flag=%d\r\n", flag);

	start = NAK;
	if (init) start = NAKCRC;
	
	while (TRUE)
	{
		/*
		if (flag == 3)
			dump('S', start);
		*/
		Send(start);
		ch = Recv(3);	/* recv with 3s timeout */
		if (ch == -1)
		{
			while(TRUE)
			{
				ch = Recv(1);
				if (ch == -1) break;;
			}
			continue;
		}

		/*
		if (flag == 3)
			dump('R', ch);
		*/

		blklen = 128;
		switch(ch)
		{
			case SOH:
				blklen = 128;
				break;
			case STX:
				blklen = 1024;
				break;
			case 'r': 		/* command:rb\r */
				Recv(1);	/* remove 'b' */
				Recv(1);	/* remove '\r' */
				continue;
			case EOT:
				/*sa_puts("rEOT\r\n");*/
				return -EOT;
			case CTRLC:
				/*sa_puts("rCTRLC\r\n");*/
				return -CTRLC;
			default:
				continue;
		}

		for (i = 0; i < blklen + 4; i++)
		{
			buf[i] = Recv(1);
		}

		if (flag == 3)
		{
			dump('R', ch);
			dump('R', 254);
			dump('B', buf[0]);
			dump('B', buf[1]);
			dump('B', prvblk & 0xFF);
			dump('B', (prvblk + 1) & 0xFF);
		}

		/*
		xprintf("\r\n%d\r\n", blklen);
		for (i = 0; i < 6; i++)
		{
			xprintf("%d %02X %c\r\n", i, buf[i], buf[i]);
		}
		for (i = blklen - 2; i < blklen + 4; i++)
		{
			xprintf("%d %02X %c\r\n", i, buf[i], buf[i]);
		}
		sa_puts("\r\n");
		*/

		blkno = buf[0];
		if (blkno != (buf[1] ^ 0xFF))
		{
			continue;
		}
		start = NAK;
		
		/*xprintf("%d %d\r\n", blkno, (prvblk + 1) & 0xFF);*/
		if (blkno != ((prvblk + 1) & 0xFF))
		{
			continue;
		}
		
		/*xprintf("blk %d ok\r\n", blkno);*/

		/* crc */
		crc1 = buf[blklen + 2]; /* crc high byte */
		crc2 = buf[blklen + 3]; /* crc low byte */

		crc = 0L;
		for (i = 0; i < blklen; i++)
		{
			crc = crc16(buf[i+2], crc);
		}

		/*
		xprintf("BLKNO: %02X %02X\r\n", blkno, buf[1]);
		printf("CRC12: %02X %02X\r\n", crc1, crc2);
		xprintf("CRC C: %02X %02X\r\n", (int)(crc >> 8), (int)(crc & 0xFF));
		*/

		if (crc1 != (crc >> 8) || crc2 != (crc & 0xFF))
		{
			continue;
		}
		xprintf("crc ok\r\n");
	
		*len = blklen;
		return blkno;
	}

}

/****************************************************************************/

BOOL RecvFiles(drive)
	char drive;
{
	int ch;
	int blkcnt, prvblk, blklen, i;
	int result;
	BOOL abort, init;
	char name[12+1];
	FILE *fp;
	int noerr;
	int flag;
	long filesize;

	flag = -1;
	while(TRUE)
	{
		flag++;
		xprintf("next file %d\r\n", flag);
		
		ch = Purge();
		if (ch == CTRLC) return FALSE;
		
		/* recv block 0 = filename */
		init = TRUE;
		result = RecvX(init, blkbuf, 0xFF, &blklen, flag);
		xprintf("result=%02X blklen=%d\r\n", result, blklen);
		if (result == -EOT || result == -CTRLC)
		{
			Send(ACK);
			return FALSE;
		}
		
		for (i = 0; i < 12; i++)
		{
			name[i] = blkbuf[i + 2];
			if (blkbuf[i + 2] == 0) break;
		}
		xprintf("name='%s'\r\n", name);
		if (name[0] == 0)
		{
			Send(ACK);
			return FALSE;
		}

		fp = fopen(name, "w");
		if (fp == NULL)
		{
			xprintf("error writing %s\r\n", name);
			return FALSE;
		}
		Send(ACK);

		blkcnt = 0;
		prvblk = 0;
		abort = FALSE;
		filesize = 0L;

		while(TRUE)
		{
			result = RecvX(init, blkbuf, prvblk, &blklen, flag);
			xprintf("result=%02X blklen=%d\r\n", result, blklen);
			if (-result == EOT)
			{
				/*sa_puts("EOT1\r\n");*/
				Send(NAK);
				ch = Recv(3);
				xprintf("EOT1/NAK ch=%02X\r\n", ch);
				if (ch == EOT)
				{
					sa_puts("EOT2/ACK\r\n");
					Send(ACK);
				}
				break;	/* next file */
			}
			else if (-result == CTRLC)
			{
				sa_puts("CTRL-C\r\n");
				Send(ACK);
				return FALSE;
			}
			prvblk = result;

			init = FALSE;
			/*xprintf("prvblk=%d, blklen =%d\r\n", prvblk, blklen);*/

			blkcnt++;

			i = fwrite(blkbuf + 2, 1, blklen, fp);
			xprintf("fwrite=%d\r\n", i);
			if (i != blklen)
			{
				Send(CTRLC);
				abort = TRUE;
				break;
			}
			filesize += blklen;
			Send(ACK);
		}
		fclose(fp);
		xprintf("size=%ld\r\n", filesize);
		if (abort) break;
	}
	
	if (abort)
	{
		cputs("RecvX error/abort\r\n");
	}
	return TRUE;
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

	rept = FALSE;
	while(TRUE)
	{
		if (!rept) blkno++;

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
		if (ch == ACK) break; /* naechster Block */
		if (ch == CTRLC) return -1;
		
		/* repeat block */
		rept = TRUE;
	}
	return blkno;
}

/****************************************************************************/

BOOL SendFile(name)
	char *name;
	
{
	FILE *fp;
	int ch;
	int blkno, blklen;
	int cnt, i;
	BOOL success, abort;

	fp = fopen(name, "r");
	if (fp==NULL) return FALSE;

	ch = Purge();
	if (ch == CTRLC) return FALSE;

	/*xprintf("purge recv %02X\r\n", ch);*/

	/* wait for transfer start (NAK)*/
	while(TRUE)
	{
		ch = Recv(1);
		if (ch == CTRLC) return FALSE;
		if (ch == NAK) break;
	}

	/*xprintf("NAK recv %02X\r\n", ch);*/

	abort = FALSE;
	blklen = 128;
	blkno = 0;
	
	while(TRUE)
	{
		cnt = 0;
		for (i = 0; i < blklen; i++)
		{
			ch = getc(fp);
			if (ch == EOF) break;
			blkbuf[i] = ch;
			cnt++;
		}
		if (ch == EOF)
		{
			abort = FALSE;
			break;
		}
		/*
		if (ch == -1)
		{
			abort = TRUE;
			break;
		}
		*/

		blkno = SendX(blkbuf, blkno);
		xprintf("blkno = %d\r\n", blkno);
		if (blkno == -1)
		{
			cputs("SendX error/abort\r\n");
			abort = TRUE;
			break;
		}
	}

	fclose();

	Send(EOT);
	ch = Recv(3);
	if (ch != ACK)
	{	/* no ACK on EOT... */
		abort = TRUE;
	}

	if (abort)
	{
		cputs("aborted\r\n");
		return FALSE;
	}
	
	cprintf("\r\n%ld bytes sent\r\n", blkno * 128L);
	return TRUE;
}

/****************************************************************************/

main()
{
	int ch;
	int drive;
	
	SetBios();
	crcinit();

	sa_puts("\r\nYModem\r\n");
	xprintf("YModem %d\r\n", 1234);

	Send('A');
	Send('B');
	Send('C');
	dump('X', 192);
	Send(CR);
	Send(LF);

	/*
	ch = Recv(10);
	printf("%d\r\n", ch);
	*/

	drive = 0;

	/*RecvFile("test.com");*/
	RecvFiles(drive);
	
	cprintf("\r\n** ende **\r\n");
}

/****************************************************************************/
