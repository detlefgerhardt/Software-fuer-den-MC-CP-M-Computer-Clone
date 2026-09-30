/****************************************************************************/
/*
	XMODEM.C  *dg*  09/2026 for MI-C compiler and CP/M
	
	getpc/putpc/Xmodem implementation
	
	Uses BIOS conout/conin/const for fast xmodem transfer
	
*/
/****************************************************************************/
/* XMODEM version history

 21.09.2026 *dg* First test version (proof of concept) to test if receive 
                 function in C via BIOS is fast enough for 19200 baud without
				 handshake - works!
 
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

;SIOADR	EQU	020H	; SIO port A data register
;SIOACR	EQU	021H	; SIO port A control register

; UART
;XMTMASK EQU 1		; MASK TO ISOLATE XMIT READY BIT
;XMTRDY  EQU 1		; VALUE WHEN READY
;RCVMASK EQU 2		; MASK TO ISOLATE RECEIVE READY BIT
;RCVRDY	EQU	2		; BIT ON WHEN READY

; Monitor
;MCONIN	EQU 0F003H
;MCONOUT EQU 0F009H
;MCONST	EQU 0F012H

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
#define CTRLC 0x03
#define LF 0x0A
#define CR 0x0D

FILE *fplog;

char buffer[1024];

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

BOOL RecvFile(name)
	char *name;
{
	int ch;
	FILE *fp;
	int blkno, blknoc, prvblk, blklen;
	int chksum, chk;
	int i;
	BOOL err, restrt;

	fp = fopen(name, "w");
	if (fp==NULL) return;
	
	prvblk = 0;
	restrt = TRUE;
	err = FALSE;

	Send(NAK);
	
	while(TRUE)
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
				if (ch == -1) break;
			}
			Send(NAK);
			err = FALSE;
			restrt = TRUE;
			continue;
		}

		if (ch == CTRLC)
		{
			Send(ACK);
			fprintf(fplog, "CTRL-C\r\n");
			break;
		}

		if (ch == EOT)
		{
			Send(ACK);
			fprintf(fplog, "EOT\r\n");
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
			fprintf(fplog,"blkno=%02X %02X error\r\n",blkno, blknoc);
			err = TRUE;
			continue;
		}
		
		/* recv block */
		
		chk = 0;
		for (i = 0; i < blklen; i++)
		{
			ch = Recv(1);
			buffer[i] = ch;
			chk = (chk + ch) & 0xFF;
		}
		
		/* chksum */
		chksum = Recv(1);

		fprintf(fplog,"blkno=%02X %02X\r\n",blkno, blknoc);
		
		/*
		for (i = 0; i< 128; i++)
		{
			fprintf(fplog, "%3d %02X\r\n", i, buffer[i]);
		}
		*/
		fprintf(fplog, "chk %02X %02X\r\n", chksum, chk);
		
		if (chksum != chk)
		{
			err = TRUE;
			continue;
		}
		
		if (((prvblk + 1) % 0xFF) == blkno)
		{
			/* sequence ok */
			prvblk = blkno;
			fwrite(buffer, blklen, 1, fp);
			/* write block */
		}
		
		Send(ACK);
		restrt = TRUE; /* next block */
	}
	
	fclose(fp);
}

/****************************************************************************/

BOOL SendFile(name)
	char *name;
	
{
	int ch;
	FILE *fp;
	int blkno, prvblk, blklen;
	int chk;
	int i, cnt;
	BOOL err, rept;
	
	fp = fopen(name, "r");
	if (fp == NULL) return FALSE;

	blklen = 128;
	blkno = 0;

	ch = Purge();
	if (ch == CTRLC) goto abort;

	while(TRUE)
	{
		ch = Recv(1);
		if (ch == CTRLC) goto abort;
		if (ch == NAK) break;
	}

	fprintf(fplog, "NAK recv\r\n");

	rept = FALSE;
	while(TRUE)
	{
		memset(buffer, 0x00, blklen);
		cnt = 0;
		for (i = 0; i < blklen; i++)
		{
			ch = getc(fp);
			if (ch == EOF) break;
			buffer[i] = ch;
			cnt++;
		}

		fprintf(fplog, "cnt=%d\r\n", cnt);
		
		if (cnt == 0)
		{	/* eof */
			Send(EOT);
			ch = Recv(3);
			if (ch != ACK)
			{	/* no ACK on EOT... */
			}
			break;
		}

		if (!rept) blkno++;

		fprintf(fplog, "blkno=%d\r\n", blkno);
		
		if (blklen == 128)
			Send(SOH);
		else
			Send(STX);
		
		Send(blkno);
		Send(blkno ^ 0xFF);
		
		chk = 0;
		for (i = 0; i < blklen; i++)
		{
			ch = buffer[i];
			Send(ch);
			chk = (chk + ch) & 0xFF;
		}
		
		Send(chk);
		
		rept = FALSE;
		
		ch = Recv(4);
		fprintf(fplog, "resp=%02X\r\n", ch);
		if (ch == ACK) continue; /* naechster Block */
		
		if (ch == CTRLC) goto abort;
		
		/* repeat block */
		rept = TRUE;
	}

	return TRUE;
	
abort:
	fclose(fp);
	return FALSE;
}

/****************************************************************************/

main()
{
	int ch;
	
	SetBios();

	printf("start\r\n");
	Recv(10);
	printf("ok\r\n");
	return;
	
	fplog = fopen("xmodem.log", "wa");
	if (fplog==NULL) return;
	
	fprintf(fplog,"---start---\r\n");

	Send('A');
	Send('B');
	Send('C');
	Send(CR);
	Send(LF);

	ch = Recv(10);
	printf("%d\r\n", ch);

	RecvFile("test.com");
	/*SendFile("test.com");*/
	
	fprintf(fplog,"---end---\r\n");
	fclose(fplog);
}

/****************************************************************************/
