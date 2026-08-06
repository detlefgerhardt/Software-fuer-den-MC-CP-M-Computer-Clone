/****************************************************************************/
/*
	STARTRAD.C  *dg*  07/2026 for MI-C compiler and CP/M
	
	STARTRADERS was written in Altair Basis in 1977. The comment in the 
	Basic programm says "MODIFIED FOR "'ALTAIR BASIC 4.0' BY - S J SINGER".
	On the help screen the program is named "STAR LANES".

	I modified the program for Commodore PET in 1983 and for C64 in 1987.
	I also added computer players.
	
*/
/****************************************************************************/

#include "stdio.h"

#define FALSE 0
#define TRUE 1
#define WORD unsigned int
#define BOOL int

#define FLD_X 12
#define FLD_Y 9

#define GRAPH TRUE

#define X_START 2
#define X_END 79
int x_side = X_START + 4 * FLD_X + 2;


#define DEBUG TRUE


#define MSG_LIN 21 /* y-pos of first inverse message bar */

char *announce = "Sondermeldung! ";
char *currency = "AD"; /* Altair Dollar */

#define COMP_CNT 5
char *comp_name[COMP_CNT] =
{
	"Altair Starways", "Beteigeuze Ltd.", "Capella Freight Co.",
	"Denebola Shippers", "Eridani Expediters"
};
int comp_value[COMP_CNT];
/*long comp_sold[COMP_CNT];*/

#define PLAY_MAX 6
#define PLAY_NAME_LEN 10
int play_cnt;
char play_name[PLAY_MAX][PLAY_NAME_LEN+1];

BOOL play_isauto[PLAY_MAX];
long play_money[PLAY_MAX];
long play_shares[PLAY_MAX][COMP_CNT];

char field[FLD_X][FLD_Y];
#define FLD_FREE 0 /* empty field */
#define FLD_SPST 1 /* space station */
#define FLD_STAR 2 /* star */
#define FLD_COMP 3 /* company */

#define FLD_CHR_CNT 8
char *field_char = " +*ABCDE";

#define DIR_CNT 4
int rx[] = {0,1,0,-1};
int ry[] = {-1,0,1,0};

#define MOVE_CNT 5
char move_x[MOVE_CNT];
char move_y[MOVE_CNT];

#define BUFFER_MAX 80
char buffer[BUFFER_MAX + 1];

#define DEBBUF_MAX 80
char debbuf[DEBBUF_MAX + 1];

unsigned rand_seed;

int round;

BOOL exit_game;
BOOL ask_exit_active;

/****************************************************************************/

/*

more company names:

Andromeda
Aldebaran
Atlas
Aquarius
Aries
Bellatrix
Bosona

Canopus
Capella
Copernicus
Capricorn
Cassiopheia

Denebola
Dalim
Delta Tauri
Daneb

Electra
Ebla
Epsilon
Eta Ceti

Denebola & Partner
Aries Trading
Electra International
Corporation
Union
Line
Star Line
Star
Company
Shipping

*/

/****************************************************************************/

extern char *InputStr();

/****************************************************************************/

unsigned int BiosAddr()
{
	#ASM
	LD HL,(0001H)	; BIOS address
	DEC HL
	DEC HL
	DEC HL
	#ENDASM
}

/****************************************************************************/

IsRunCpm()
{
	return BiosAddr() == 0XEE00;
}

/****************************************************************************/
/* BIOS function
/* inputs char from console */

/*
BI_CoIn()
{
	#ASM
	LD HL,(0001H)	; BIOS address
	LD DE,0006H
	ADD HL,DE
	LD DE,conin1
	PUSH DE
	JP (HL)		; conin
conin1:
	LD L,A
	LD H,0
	#ENDASM	
}
*/

/****************************************************************************/
/* BIOS function
/* returns 0xFF if console character is available */

/*
int BI_CoSt()
{
	#ASM
	LD HL,(0001H)	; BIOS address
	LD DE,0003H
	ADD HL,DE
	LD DE,const1
	PUSH DE
	JP (HL)		; const
const1:
	LD L,A
	LD H,0
	#ENDASM	
}
*/

/****************************************************************************/
/* funktioniert nicht ? */

SI_Init()
{
#ASM	
	LD C,0F1h
	LD B,8
	LD HL,SI_ITab
	OTIR
	RET

SI_ITab:
	DEFB 1,0
	DEFB 3,11100001B	; -CTS UND -DCD ENABLE rev 3.4 term1
	DEFB 4,01001100B
	DEFB 5,11101010B	; dtr rts +12V bedeutet ready rev 3.4
	
#ENDASM
}

/****************************************************************************/

SI_Out(ch)
	int ch;
{
#ASM
	POP BC
	POP HL
	PUSH HL
	PUSH BC
	
SI_O1:
	IN A,(0F1h)
	AND 04h
	JR Z,SI_O1

	LD A,L
	OUT (0F0h),A
#ENDASM
}

/****************************************************************************/

SI_PutStr(s)
	char *s;
{
#if FALSE	
	if (IsRunCpm()) return;
	
	while(*s)
		SI_Out(*s++);
#endif
}		

/****************************************************************************/

int BD_CoIn()
{
	return BDOS(0xFF, 6);
}

/****************************************************************************/

BD_CoOut(ch)
	char ch;
{
	BDOS(ch, 6);
}

/****************************************************************************/

int pow2(x)
	int x;
{
	int y, i;
	
	if (x==0) return 1;
	
	y = 1;
	for (i = 0; i < x; i++)
		y *= 2;
	return y;
}
/****************************************************************************/
/* integer division with aufrunden */

long div(x,y)
	long x,y;
{
	if (y==0) return 0L;
	if (x % 2 == 0)
		return x / y;
	else
		return x / y + 1;
}

/****************************************************************************/
/* division / 2 with round */

/*
long div2(x)
	long x;
{
	if (x % 2 == 0)
		return x / 2;
	else
		return x / 2 + 1;
}
*/

/****************************************************************************/

int strcmpi(s1, s2)
	char *s1,*s2;
{
	char ch1,ch2;

	while(*s1)
	{
        ch1 = tolower(*s1);
        ch2 = tolower(*s2);
        if (ch1 != ch2) return ch1 - ch2;
		s1++;
		s2++;
    }
    return 0;
}


/****************************************************************************/

int strncmpi(s1, s2, n)
	char *s1,*s2;
	int n;
{
	int i;
	char ch1,ch2;
	
    for (i = 0; *s1 && i < n; i++)
	{
        ch1 = tolower(*s1);
        ch2 = tolower(*s2);
        if (ch1 != ch2) return ch1 - ch2;
		s1--;
		s2--;
    }
    return 0;
}

/****************************************************************************/

SeedRefresh()
{
	#ASM
	LD A,R
	LD L,A
	LD H,0
	#ENDASM	
}	

/****************************************************************************/

RandNext()
{
	int lsb;

    lsb = rand_seed & 1;
    rand_seed >>= 1;

    if (lsb != 0)
	{
		/* 15-Bit-LFSR, Polynom: x^15 + x^14 + 1
           Maske bleibt innerhalb von 15 Bit.
        */
		rand_seed ^= 0x6000;
	}
	return rand_seed;
	
}

/****************************************************************************/

Rand(s, n)
	unsigned int s,n;
{
	long rnd, x, rng;

	rnd = RandNext();
	x = ((n - s + 1) * rnd) / 32767 + s;

	return x;
}

/****************************************************************************/

char *strtolower(s)
	char *s;
{
	while(*s)
	{
		*s = tolower(*s);
		s++;
	}
}

/****************************************************************************/

int GetChr()
{
	return BD_CoIn();
}

/****************************************************************************/

PutChr(ch)
	char ch;
{
	BD_CoOut(ch);
}

/****************************************************************************/

PutStr(s)
	char *s;
{
	int i;
	while(*s)
		BD_CoOut(*s++);
}		

/****************************************************************************/

ScrClr()
{
	PutStr("\033[2J");
	PutStr("\033[H");
}

/****************************************************************************/

ScrHome()
{
	PutStr("\033[H");
}

/****************************************************************************/
/* x = 1..80
   y = 1..25
*/

ScrPos(x,y)
	int x,y;
{
	printf("\033[%d;%dH", y, x);
}

/****************************************************************************/

ScrInv()
{
	PutStr("\033[7m");
}

/****************************************************************************/

ScrEmph()
{
	/* yellow */
	PutStr("\033[33m");
	/* bright */
	PutStr("\033[1m");
}

/****************************************************************************/

ScrBlink()
{
	PutStr("\033[5m");
}

/****************************************************************************/

ScrNorm()
{
	PutStr("\033[0m");
}

/****************************************************************************/
/* switch to special graphics characters set and line drawing set */

GrOn()
{
	PutStr("\033(0");
}

/****************************************************************************/
/* switch to US ASCII */

GrOff()
{
	PutStr("\033(B");
}

/****************************************************************************/
/* print string with control codes
	001 = normal
	002 = invers
	003 = emph
	004 = graph
	005 = graph off
*/

PrnStr(s)
	char *s;
{
	while(*s)
	{
		switch(*s)
		{
			case 001:
				ScrNorm();
				break;
			case 002:
				ScrInv();
				break;
			case 003:
				ScrEmph();
				break;
			case 004:
				GrOn();
				break;
			case 005:
				GrOff();
				break;
			default:
				PutChr(*s);
		}
		s++;
	}
	ScrNorm();
}

/****************************************************************************/

ClrMsg()
{
	int x,y;
	
	ScrNorm();
	for (y = 0; y < 3; y++)
	{
		ScrPos(X_START, MSG_LIN + y);
		for (x = X_START; x < X_END; x++)
			PutChr(' ');
	}
}

/****************************************************************************/

ProgExit()
{
	ScrNorm();
	GrOff();
	ScrClr();
	_EXIT();
}

/****************************************************************************/

BOOL AskExitGame()
{
	BOOL result;
	
	ClrMsg();
	ask_exit_active = TRUE;
	while(TRUE)
	{
		InputStr(X_START, 24, "Spiel beenden (j/n) ? ",buffer,1);
		buffer[0] = tolower(buffer[0]);
		if (buffer[0] == 'j')
		{
			exit_game = TRUE;
			result = TRUE;
			ProgExit();
			break;
		}
		if (buffer[0] == 'n')
		{
			result = FALSE;
			break;
		}
	}
	
	ask_exit_active = FALSE;
	return result;
}

/****************************************************************************/

char *InputStr(x,y, prompt, buffer, len)
	int x;
	int y;
	char *prompt;
	char *buffer;
	int len;
{
	int xp, i;
	char c;
	int sp = 0;

	xp = x;
	if (prompt != NULL)
	{
		ScrPos(x, y);
		PutStr(prompt);
		PutChr(' ');
		xp = x + strlen(prompt) + 1;
	}

	ScrPos(xp, y);
	for (i = 0; i<len; i++)
		PutChr(' ');
	ScrPos(xp, y);

	buffer[0] = '\0';
	
	ScrEmph();
	while(TRUE)
	{
		/*if (chrrdy())*/
		/*if (BI_CoSt())*/
		c = BD_CoIn();
		if (c != 0)
		{
			/*c = BI_CoIn();*/
			/*printf("*%d*", c);*/
			if (!ask_exit_active && c == 3)
			{	/* Ctrl-C */
				if (AskExitGame())
				{
					buffer[0] = '\0';
					break;
				}
			}
			if (c==13) break;
			if ((c==08 || c==127) && sp > 0)
			{
				sp--;
				buffer[sp] = '\0';
				ScrPos(xp+sp, y);
				PutChr(' ');
				ScrPos(xp+sp,y);
			}
			else if (c>=32 && c<127 && sp<len)
			{
				buffer[sp++] = c;
				buffer[sp] = '\0';
				PutChr(c);
			}
		}
	}
	
	ScrNorm();
	return buffer;
}

/****************************************************************************/

#if FALSE
Line()
{
	int i;
	
	ScrInv();
	for (i=0; i<40; i++)
		PutChr(' ');
	ScrNorm();
}
#endif

/****************************************************************************/

int GetFreeFields()
{
	int x,y;
	int cnt = 0;
	
	for(y=0; y<FLD_Y; y++)
		for(x=0; x<FLD_X; x++)
			if (field[x][y] == FLD_FREE) cnt++;
		
	return cnt;
}

/****************************************************************************/

HeadLine()
{
	int len, pos, i;
	int free = GetFreeFields();

	len = X_END - X_START + 1;
	for(i = 0; i < len; i++)
		buffer[i] = ' ';
	buffer[len] = '\0';

	sprintf(buffer + 1, "S T A R  T R A D E R S   by   *dg*");

	sprintf(buffer + len - 30, "freie Felder: %3d  Runde: %3d", free, round);
	/*sprintf(buffer, " S T A R  T R A D E R S  (%3d) Runde:%3d ", free, round);*/

	/* replace '\0' from sprintf */
	/*
	for(i = 0; i < len; i++)
		if (buffer[i] == '\0')
			buffer[i] = ' ';
	*/
	
	ScrPos(X_START, 1);
	ScrInv();
	
	for(i = 0; i < len; i++)
	{
		if (buffer[i] == '\0')
			PutChr(' ');
		else
			PutChr(buffer[i]);
	}

	/*
	pos = X_START + len;
	for (i = pos; i <= X_END; i++)
		PutChr(' ');
	*/
	ScrNorm();
}

/****************************************************************************/
/* centered message in inverse at line y */
/* control characters */

MsgLine(msg, y, inv)
	char *msg;
	int y;
	BOOL inv;
{
	int i, p, len;
	char buffer[81];

	/*
	ScrPos(1,1);
	printf("ml0 strlen=%d buf=%u msg=%u %s", strlen(msg), buffer, msg, msg);
	*/

	len = X_END - X_START + 1;
	for (i = 0; i < len; i++)
		buffer[i] = ' ';
	buffer[len] = '\0';

	/*
	ScrPos(1,2);
	printf("ml1 len=%d strlen=%d buf=%u %u %s", len, strlen(msg), buffer, msg, msg);
	getchar();
	*/

	if (msg != NULL)
	{
		p = (len - strlen(msg)) / 2;
		
		/*
		ScrPos(1,1);
		printf("ml2 p=%d %d", p, strlen(msg));
		getchar();
		*/
		
		for(i = 0; i < strlen(msg); i++)
			buffer[p + i] = msg[i];
	}
	
	ScrPos(X_START, y);
	if (inv) ScrInv();
	PrnStr(buffer);
	ScrNorm();
	
	ScrPos(X_START, y);
}

/****************************************************************************/

BottomLine(msg)
	char *msg;
{
	MsgLine(msg, MSG_LIN + 3, TRUE);
}

/****************************************************************************/

WaitSpace()
{
	int ch;
	
	BottomLine("weiter mit SPACE-Taste");
	while(TRUE)
	{
		ch = BD_CoIn();
		if (ch == ' ') return;
		if (!ask_exit_active && ch == 3)
		{	/* Ctrl-C */
			if (AskExitGame()) return;
		}
	}
}

/****************************************************************************/

ShowCell(x, y, inv)
	int x, y;
	BOOL inv;
{
	if (inv) ScrInv();
	ScrPos(X_START + 1 + x * 4, 3 + y * 2);
	PutChr(' ');
	PutChr(field_char[field[x][y]]);
	PutChr(' ');
	if (inv) ScrNorm();
}		

/****************************************************************************/

ShowFrame()
{
	int i;
	
	/* Titelzeile */

	ScrHome();
	HeadLine();
	
	ScrPos(1, 2);
/*          0        1         2         3         4         5         6         7 */
/*	PutStr("1234567890123456789012345678901234567890123456789012345678901234567890123456789"); */

	/* Sidebar */

	ScrPos(x_side,5);
	PrnStr("\004qu\005Barschaft\004tqqq\005");

	ScrPos(x_side,8);
	PrnStr("\004qu\005Aktien\004tqqqqqq\005");
	
	ScrPos(x_side,9);
	PrnStr("    Anzahl Wert");
	ScrPos(x_side,15);
	PrnStr("\004qu\005Summen\004tqqqqqq\005");
	ScrPos(x_side,18);
	PrnStr("\004qqqqqqqqqqqqqqq\005");
	ScrPos(x_side,20);
	PrnStr("\004qqqqqqqqqqqqqqq\005");

	BottomLine(NULL);
}

/****************************************************************************/

ShowMap()
{
	int x,y;

#ifndef GRAPH
	for(y = 0; y <= FLD_Y; y++)
	{
		ScrPos(X_START, 2 + y * 2);
		for(x = 0; x < FLD_X; x++)
		{
			PutStr("+---");
		}
		PutChr('+');

		if (y < FLD_Y)
		{
			ScrPos(X_START, 3 + y * 2);
			for (x = 0; x < FLD_X; x++)
			{
				PutChr('!');
				PutChr(' ');
				PutChr(field_char[field[x][y]]);
				PutChr(' ');
			}
			PutChr('+');
		}
	}
#else
	GrOn();
	for(y = 0; y <= FLD_Y; y++)
	{
		ScrPos(X_START, 2 + y * 2);
		if (y == 0)
		{
			for(x = 0; x < FLD_X; x++)
			{
				if (x == 0)
					PutChr('\154');
				else
					PutChr('\167');
				PutStr("\161\161\161");
			}
			PutChr('\153');
		}
		else if (y < FLD_Y)
		{
			for(x = 0; x < FLD_X; x++)
			{
				if (x == 0)
					PutChr('\164');
				else
					PutChr('\156');
				PutStr("\161\161\161");
			}
			PutChr('\165');
		}
		else
		{
			for(x = 0; x < FLD_X; x++)
			{
				if (x == 0)
					PutChr('\155');
				else
					PutChr('\166');
				PutStr("\161\161\161");
			}
			PutChr('\152');
		}

		if (y < FLD_Y)
		{
			ScrPos(X_START, 3 + y * 2);
			for(x = 0; x < FLD_X; x++)
			{
				PutStr("\170   ");
			}
			PutChr('\170');
		}
	}
	GrOff();
	
	for (y = 0; y < FLD_Y; y++)
		for (x = 0; x < FLD_X; x++)
			if (field[x][y] != FLD_FREE)
				ShowCell(x, y, FALSE);

#endif

}

/****************************************************************************/

ShowSidebar(pl, co)
	int pl, co;
{
	int c;
	long shares, money;
	char *name;
	
	ScrEmph();
	ScrPos(x_side + 1,3);
	printf("%-9s", play_name[pl]);
	
	ScrPos(x_side, 6);
	printf("%9ld %-2s", play_money[pl], currency);

	shares = 0L;
	money = play_money[pl];
	for(c = 0; c < COMP_CNT; c++)
	{
		if (comp_value[c] == 0) continue;
		ScrPos(x_side, 10 + c);
		if (co != -1 && c==co) ScrInv();
		name = comp_name[c];
		printf("%c %8ld %4d", comp_name[c][0], play_shares[pl][c], comp_value[c]);
		if (c==co) ScrNorm();
		shares += play_shares[pl][c];
		money += comp_value[c] * play_shares[pl][c];
	}

	ScrPos(x_side + 1, 16);
	printf("%9ld Akt", shares);
	
	ScrPos(x_side + 1, 17);
	if (money < 100000000)
		printf("%9ld %-2s", money, currency);
	else
	{
		printf("%5ld.%03ld Mio", div(money, 1000000L), money & (1000000L - 1));
	}
	
	ScrNorm();
}

/****************************************************************************/

ShowMerge(co1, co2)
	int co1, co2;
{
	int pl;
	
	ScrClr();
	HeadLine();

	ScrPos(X_START, 3);
	ScrEmph();
	PrnStr(announce);
	ScrNorm();

	ScrPos(X_START, 5);
	ScrEmph();
	PutStr(comp_name[co1]);
	ScrNorm();
	PutStr(" wurde aufgeloest.");

	ScrPos(X_START, 7);
	PutStr("Sie wurde von ");
	ScrEmph();
	PutStr(comp_name[co2]);
	ScrNorm();
	PutStr(" uebernommen.");

	ScrPos(X_START + 11, 10);
	PutStr(          "! alter  ! neuer  !        ! Verguet-");
	ScrPos(X_START, 11);
	printf("Spieler    ! Anteil ! Anteil ! Gesamt ! ung/%s", currency);
	ScrPos(X_START, 12);
	PutStr("-----------+--------+--------+--------+----------");
	for (pl = 0; pl<play_cnt; pl++)
	{
		ScrPos(X_START + 11, 13 + pl);
		PutStr(     "!        !        !        !");
	}
}

/****************************************************************************/
/* get dir to neighbor that contains symbol */

int GetSymDir(mx, my, sym)
	int mx, my, sym;
{
	int d,x,y;
	
	for (d = 0; d < DIR_CNT; d++)
	{
		x = mx + rx[d];
		y = my + ry[d];
		if (x < 0 || x >= FLD_X || y < 0 || y >= FLD_Y) continue;

		if (field[x][y] == sym) return d;
	}
	return -1;
}

/****************************************************************************/
/* calc company sizes (total amount of shares */
/*
CalcCompSizes()
{
	int co,x,y;
	
	for (co=0; co<COMP_CNT; co++)
		comp_size[co] = 0;
	
	for (y=0; y<FLD_Y; y++)
		for (x=0; x<FLD_Y; x++)
		{
			co = field[x][y] - FLD_COMP;
			comp_size[co]++;
		}
}
*/

/****************************************************************************/

/*
int GetCompShares(co)
	int co;
{
	int pl;
	long size;
	
	size = 0;
	for(pl = 0; pl < play_cnt; pl++)
		size += play_shares[pl][co];
	
	return size;
}
#endif
*/

/****************************************************************************/
/* sum of sold shares of company co */

long CompSold(co)
	int co;
{
	int pl;
	long sum;

	if (comp_value[co] == 0) return 0;
	
	for (pl = 0; pl<play_cnt; pl++)
		sum += play_shares[pl][co];
	return sum;
}

/****************************************************************************/

/* x/y = bytearrays with MOVE_CNT elements */
/* return FALE: no enough free fields */

BOOL GetMovePos(cnt, xp, yp)
	int cnt;
	char xp[], yp[];
{
	int free, i, idx, p, x, y;
	BOOL found;
	int idx_l[5];
	int idx_n = 0;

	free = GetFreeFields();
	
	if (free < cnt) return FALSE;

	/* create cnt different indices into free fields */
	for (p=0; p<cnt; p++)
	{
		while(TRUE)
		{
			idx = Rand(1, free); /* index of free field */
			for (i = 0; i<idx_n; i++)
				if (idx_l[i] == idx)
				{	/* idx already in list */
					idx = -1;
					break;
				}
			if (idx != -1) break; /* valid */
		}
		
		idx_l[idx_n++] = idx;
		
		/*printf("GetMove *p=%d free=%d idx=%d*", p, free, idx);*/
		found = FALSE;
		for(y = 0; y < FLD_Y; y++)
		{
			for(x = 0; x < FLD_X; x++)
			{
				/*printf("*x=%d y=%d field=%d idx=%d*", x, y, field[x][y], idx);*/
				if (field[x][y] == FLD_FREE)
				{
					idx--;
					if (idx == 0)
					{
						found = TRUE;
						break;
					}
				}
			}
			if (found) break;
		}
		if (!found) return FALSE;
		
		/*field[x][y] = FLD_STAR;*/
		xp[p] = x;
		yp[p] = y;
		free--;
		/*printf("\r\n*p=%d x=%d y=%d*\r\n", p, x, y);*/
	}
}

/****************************************************************************/
/* returns new company or -1 */

int ChkFreeCompany()
{
	int co;
	
	for(co = 0; co < COMP_CNT; co++)
	{
		if (comp_value[co] == 0) return co;
	}
	/* no free company */
	return -1;
}

/****************************************************************************/
/* computer */
/* fusion possible */
/* returns score */

long AutoMergePossible(pl, mx, my, na)
	int pl, mx, my;
	int na[];
{
	long big_value, value, score, sold;
	int big_co;
	int co, x, y, f;
	long ak, av;
	
	long deb_sp1, deb_sp2, deb_sm, deb_ak, deb_av;

	sprintf(debbuf, "AutoMerge %d\r\n", pl);
	SI_PutStr(debbuf);
	
	/* determine biggest company -> big_co, biggest value -> big_value */
	big_value = 0;
	big_co = -1;
	ak = 0;
	for (co = 0; co < COMP_CNT; co++)
	{
		if (na[FLD_COMP + co] == 0) continue;

		value = comp_value[co];
		if (value > big_value)
		{
			big_value = value;
			big_co = co;
		}
	}

	sprintf(debbuf, " big_co=%d big_val=%d\r\n", big_co, big_value);
	SI_PutStr(debbuf);

	for (co = 0; co < COMP_CNT; co++)
	{
		if (na[FLD_COMP + co] == 0 || co == big_co) continue;

		sold = CompSold(co);

		/* debug */

		deb_sp1 = div(play_shares[pl][co], 2) * comp_value[big_co];
		deb_sp2 = 0;
		/*value = play_shares[pl][co] - div(sold, play_cnt);*/ /* rel. anteil */
		if (sold > 0)
		{
			deb_sp2 = play_shares[pl][co] * comp_value[co] / 20L;
		}

		deb_sm = play_shares[pl][co] * comp_value[co]; /* aktienverlust */
		
		sprintf(debbuf, " %d: ak=%ld sold=%ld +=%ld,%ld -=%ld\r\n",
			co, play_shares[pl][co], sold, deb_sp1, deb_sp2, deb_sm);
		SI_PutStr(debbuf);

		/*******/

		/*value = play_shares[pl][co] - div(sold, play_cnt);*/ /* rel. anteil */

		score += div(play_shares[pl][co], 2) * comp_value[co];
		if (sold > 0)
		{
			 /* profit through compensation */
			/* score += (10L * play_shares[pl][co]) / sold * value; */
			score = play_shares[pl][co] * comp_value[co] / 20L;
		}
		score -= play_shares[pl][co] * comp_value[co]; /* share minus */
	}
	
	return score;
}

/****************************************************************************/
/* computer */
/* eval player field move */
/* returns score */

long AutoEvalMove(pl, move, mx, my)
	int pl, move, mx, my;
{
	int score, shares;
	int p, co, n, b, d, x, y, f;
	int na[FLD_CHR_CNT];	/* 7 neighbor types */
	int cn; /* company count */

	score = 0;

	sprintf(debbuf,"AutoEvalMove %d\r\n", move);
	SI_PutStr(debbuf);

	/* check 4 direct neighbors */
	for (n = 0; n < FLD_CHR_CNT; n++)
		na[n] = 0;
	cn = 0;
	co = -1;
	for (d = 0; d < DIR_CNT; d++)
	{
		x = mx + rx[d];
		y = my + ry[d];
		if (x < 0 || x >= FLD_X || y < 0 || y >= FLD_Y) continue;

		f = field[x][y];
		if (f >= FLD_COMP)
		{
			co = f - FLD_COMP; /* remember company, if any */
			if (na[f] == 0) cn++; /* company count */
		}
		na[f]++; /* count neighbor type */
	}

	sprintf(debbuf, " cn=%d cp=%d\r\n", cn, co);
	SI_PutStr(debbuf);
	SI_PutStr("an[]:");
	for (n = 0; n < FLD_CHR_CNT; n++)
	{
		sprintf(debbuf, " %d", na[n]);
		SI_PutStr(debbuf);
	}
	SI_PutStr("\r\n");

	if (cn == 0 && ChkFreeCompany() >= 0)
	{
		/* no adjacent companies */
		
		if (ChkFreeCompany()==0 || na[FLD_SPST] == 0 && na[FLD_STAR] == 0)
			/* no new company */
			return 0;

		/* new company */
		if (na[FLD_SPST] > 0) score += na[FLD_SPST] * 500; /* with space station */
		if (na[FLD_STAR] > 0) score += na[FLD_STAR] * 2500; /* with star */

		sprintf(debbuf," STAR/SPST: score=%d\r\n", score);
		SI_PutStr(debbuf);
	}
	else if (cn == 1)
	{
		/* expansion possible, company in co */
		shares = play_shares[pl][co] - div(CompSold(co), play_cnt);
		if (na[FLD_SPST] > 0) score += 100 * shares;
		if (na[FLD_STAR] > 0) score += 500 * shares;
		
		sprintf(debbuf," Expand: score=%d\r\n", score);
		SI_PutStr(debbuf);
	}
	else /* cn > 1 */
	{ 
		/* fusion possible */
		score += AutoMergePossible(pl, mx, my, na);
		
		sprintf(debbuf," Merge: score=%d\r\n", score);
		SI_PutStr(debbuf);
	}
	
	return score;
}

/****************************************************************************/
/* computer */
/* returns best move 0..n-1 */

int AutoField(pl, cnt, xp, yp)
	int pl, cnt;
	char xp[], yp[];
{
	int m, move;
	long score, max_sc;

	move = -1;
	max_sc = -10000000;
	for (m = 0; m<cnt; m++)
	{
		score = AutoEvalMove(pl, m, xp[m], yp[m]);
		
		sprintf(debbuf,"AutoField %d: score = %d\r\n",
			m, score);
		SI_PutStr(debbuf);
		
		if (score > max_sc)
		{
			max_sc = score;
			move = m;
		}
	}
	if (move == -1) move = Rand(0, 4); /* random move */
	
	return move;
}

/****************************************************************************/
/* shares are splitted if value > 3000 */

ChkSplitting()
{
	int co, pl;
	
	for (co=0; co < COMP_CNT; co++)
	{
		if (comp_value[co] > 3000)
		{
			for (pl = 0; pl < play_cnt; pl++)
				play_shares[pl][co] *= 2;
			comp_value[co] = div((long)comp_value[co], 2);

			ClrMsg();
			
			ScrPos(X_START, MSG_LIN + 1);
			ScrEmph();
			PutStr(announce);
			ScrNorm();
			PutStr("Die Aktien von ");
			ScrEmph();
			PutStr(comp_name[co]);
			ScrNorm();
			PutStr(" wurden 2:1 gesplittet.");
			
			WaitSpace();
		}
	}
}

/****************************************************************************/

NewCompany(pl, mx, my, b)
	int pl, mx, my, b;
{
	int co, d, x, y;
	
	/*
	ScrPos(41,2);
	printf("New pl=%d mx=%d my=%d b=%d", pl, mx, my, b);
	getchar();
	*/
	
	co = ChkFreeCompany();
	
	if (co == -1)
	{
		/* no free company */
		field[mx][my] = FLD_SPST;
		ShowCell(mx, my, TRUE);
		return;
	}

	ScrPos(X_START, MSG_LIN);
	ScrEmph();
	PutStr(announce);
	ScrNorm();
	PutStr(" Eine neue Gesellschaft wurde gegruendet. Sie heisst");
	ScrEmph();
	ScrPos(X_START, MSG_LIN + 1);
	PutStr(comp_name[co]);
	ScrNorm();
	PutStr(".");
	
	/* set fields */
	field[mx][my] = FLD_COMP + co;
	ShowCell(mx, my, TRUE);
	
	/* look for neighbor stars and platforms */
	for (d = 0; d < DIR_CNT; d++)
	{
		x = mx + rx[d];
		y = my + ry[d];
		if (x < 0 || x >= FLD_X || y < 0 || y >= FLD_Y) continue;
		
		if (field[x][y] == FLD_SPST)
		{	/* space station */
			field[x][y] = FLD_COMP + co;
			ShowCell(x, y, TRUE);
			comp_value[co] += 100;
		}
		else if (field[x][y] == FLD_STAR)
		{	/* star */
			field[x][y] = FLD_COMP + co;
			ShowCell(x, y, TRUE);
			comp_value[co] += 500;
		}
	}

	/* player shares */
	play_shares[pl][co] = 5L;
	/*comp_sold[co] = 5L;*/
}

/****************************************************************************/
/* expand company by  stars or space stations */

ExpCompany(pl, co, mx, my)
	int pl, co, mx, my;
{
	int d, x, y;
	
	field[mx][my] = FLD_COMP + co;
	ShowCell(mx, my, TRUE);
	comp_value[co] += 100;

	/* look for addidtional neighbor stars and platforms to include */
	for (d = 0; d < DIR_CNT; d++)
	{
		x = mx + rx[d];
		y = my + ry[d];
		if (x < 0 || x >= FLD_X || y < 0 || y >= FLD_Y) continue;
		
		if (field[x][y] == FLD_SPST)
		{	/* space station */
			field[x][y] = FLD_COMP + co;
			ShowCell(x, y, TRUE);
			comp_value[co] += 100;
		}
		else if (field[x][y] == FLD_STAR)
		{	/* star */
			field[x][y] = FLD_COMP + co;
			ShowCell(x, y, TRUE);
			comp_value[co] += 500;
		}
	}
}

/****************************************************************************/
/* expand or merge companies */
/* perform */

int ExpOrMergeCompanys(pl, mx, my)
	int pl, mx, my;
{
	int d, x, y, sym;
	int co_l[DIR_CNT];
	int co_n, co, ci, p, big_co;
	long value, big_value, ak, compen;

	/* get a list of all neighbor companies -> co_l, count -> co_n */
	for (d = 0; d < DIR_CNT; d++)
		co_l[d] = 0;
	
	co_n = 0;
	for (d = 0; d < DIR_CNT; d++)
	{
		x = mx + rx[d];
		y = my + ry[d];
		if (x < 0 || x >= FLD_X || y < 0 || y >= FLD_Y) continue;
		
		if (field[x][y] > 2)
		{
			co_l[co_n++] = field[x][y] - FLD_COMP; /* 0..COMP_CNT-1 */
		}
	}
	
	if (co_n == 0) return; /* no company, something went wrong */
	
	if (co_n == 1)
	{
		/* company */
		ExpCompany(pl, co_l[0], mx, my);
		return;
	}

	/* more than one company: merge */
	
	/* determine biggest company -> big_co, biggest value -> big_value */
	big_value = 0;
	big_co = -1;
	for (ci = 0; ci<co_n; ci++)
	{
		co = co_l[ci];
		value = CompSold(co) * comp_value[co];
		if (value > big_value)
		{
			big_value = value;
			big_co = co;
		}
	}
	
	/* perform merge for each company in co_l and show table */
	for (ci = 0; ci < co_n; ci++)
	{
		co = co_l[ci];
		
		if (co == big_co) continue;
		
		ShowMerge(co, big_co);

		/* transfer all player shares to big_co */
		ScrEmph();
		for (p = 0; p<play_cnt; p++)
		{
			ak = div(play_shares[p][co], 2); /* new shares 2:1 */
			play_shares[p][big_co] += ak;
			/*comp_sold[big_co] += ak;*/
			compen = 0;
			if (CompSold(co) > 0)
			{
				/* compensation, 1/10 % vom relativen aktienbesitz */
				/* compen = (10L * (long)play_shares[p][co] / compen * (long)comp_value[co]); */
				compen = play_shares[p][co] * comp_value[co] / 20;
				play_money[p] += compen;
			}
			
			ScrPos(X_START, 13 + p);
			PutStr(play_name[p]);
			ScrPos(X_START + 13, 13 + p);
			printf("%6ld", play_shares[p][co]);
			ScrPos(X_START + 22, 13 + p);
			printf("%6ld", ak);
			ScrPos(X_START + 31, 13 + p);
			printf("%6Ld", play_shares[p][big_co]);
			ScrPos(X_START + 40, 13 + p);
			printf("%8ld", compen);
		}
		ScrNorm();
		
		comp_value[big_co] += comp_value[co];
		
		/* delete company */
		comp_value[co] = 0;
		/*comp_sold[co] = 0;*/

		WaitSpace();
	}

	/* merge is also an expantion */
	ExpCompany(pl, big_co, mx, my);

	/* restore screen */

	ScrClr();
	ShowFrame();
	ShowMap();
	ShowSidebar(pl, -1);
	
	/* big_co takes over all other companies */
	ScrInv();
	for (y = 0; y < FLD_Y; y++)
		for (x = 0; x < FLD_X; x++)
			for (ci = 0; ci < co_n; ci++)
			{
				co = co_l[ci];
				if (co != big_co &&
					field[x][y] == FLD_COMP + co)
				{
					field[x][y] = FLD_COMP + big_co;
					ShowCell(x, y, FALSE);
				}
			}
	ScrNorm();
}

/****************************************************************************/
/* eval player field move */

EvalMove(pl, mx, my)
	int pl, mx, my;
{
	int p, b, d, x, y;

	/* check 4 direct neighbors */
	b = 0;
	for (d = 0; d < DIR_CNT; d++)
	{
		x = mx + rx[d];
		y = my + ry[d];
		if (x < 0 || x >= FLD_X || y < 0 || y >= FLD_Y) continue;
		
		b = b | pow2(field[x][y]);
	}

	if (b == 1)
	{	/* no neighbor */
		field[mx][my] = FLD_SPST;
		ShowCell(mx, my, TRUE);
		return;
	}
	
	b &= 0xFE;
	if (b==2 || b==4)
	{	/* one or more station or star */
		NewCompany(pl, mx, my, b);
		WaitSpace();
	}
	else
	{	/* one or more company */
		ExpOrMergeCompanys(pl, mx, my);
	}
}

/****************************************************************************/
/* player field move (select position 1..5) */

PlayField(pl)
	int pl;
{
	int p, mov;
	BOOL ok;

#ifdef DEBUG
	if (round == 1)
		ok = SetMoves(5, move_x, move_y);
	else
		ok = GetMovePos(5, move_x, move_y);
#else	
	ok = GetMovePos(5, move_x, move_y);
#endif

	SI_PutStr("PlayField\r\n");
	
	ScrInv();
	for (p = 0; p<5; p++)
	{
		ScrPos(X_START + 1 + move_x[p] * 4, 3 + move_y[p] * 2);
		PutChr(' ');
		PutChr(p + '1');
		PutChr(' ');
		sprintf(debbuf, " %d: %d %d\r\n", p, move_x[p], move_y[p]);
		SI_PutStr(debbuf);
	}
	ScrNorm();

	if (!play_isauto[pl])
	{
		MsgLine("\0031\001-\0035\001 erlaubte Zuege", MSG_LIN + 1, FALSE);

		while(TRUE)
		{
			InputStr(x_side + 1, 19, "Zug:", buffer, 1);
			if (exit_game) return;
			mov = atol(buffer);
			if (mov >= 1 && mov <= 5) break;
		}
		
		mov--;
	}
	else
	{
		WaitSpace();
		mov = AutoField(pl, 5, move_x, move_y);
		WaitSpace();
	}
	
	/* clear unused moves */
	for (p=0; p<5; p++)
	{
		if (p != mov)
		{
			ScrPos(X_START + 1 + move_x[p] * 4, 3 + move_y[p] * 2);
			PutStr("   ");
		}
	}

	EvalMove(pl, move_x[mov], move_y[mov]);
	
	ChkSplitting();

	/*
	ScrPos(1,3);
	printf("PF3");
	getchar();
	*/
	
	return;
}

/****************************************************************************/
/* computer */

TradeAuto(pl)
	int pl;
{
	int idx[COMP_CNT], value[COMP_CNT];
	long k[2][COMP_CNT]; /* 0=sell, 1=buy */
	long money, shares;
	int co, ci, s1, s2, xc, cn, sb;

	/*
	ScrPos(1,1);
	printf("TA1");
	getchar();
	*/

	for (co = 0; co<COMP_CNT; co++)
	{
		k[0][co] = 0;
		k[1][co] = 0;
	}

	/* sort shares for value ascending */
	cn = 0;
	for (co	= 0; co<COMP_CNT; co++)
	{
		idx[co] = co;
		value[co] = comp_value[co] > 0 ? comp_value[co] : 4000;
		if (comp_value[co] > 0) cn++;
	}
	
	if (cn = 0) return; /* no companies */
	
	for (s1 = 0; s1 < COMP_CNT - 1; s1++)
		for (s2 = s1 + 1; s2 < COMP_CNT; s2++)
		{
			if (value[s2] < value[s1])
			{
				xc = idx[s1];
				idx[s1] = idx[s2];
				idx[s2] = xc;
				xc = value[s1];
				value[s1] = value[s2];
				value[s2] = xc;
			}
		}

	/*
	for (ci = 0; ci < 5; ci++)
	{
		co = idx[ci];
		ScrPos(1, ci + 1);
		printf("%d: co=%d %4d ", ci, co, value[ci]);
	}
	getchar();
	*/
	
	money = play_money[pl];

	/* sell */
	if (cn != 1 && comp_value[idx[2]] != comp_value[idx[cn-1]])
	{
		/* sell all wihout biggest */
		for (ci = cn-1; ci >= 1; ci--)
		{
			co = idx[ci];
			if (play_shares[pl][co] * comp_value[co] > 0)
			{
				k[0][co] = - play_shares[pl][co];
				money = money - k[0][co] * comp_value[co];
			}
		}
	}
	
	/* buy */
	for(ci = 1; ci < cn; ci++)
	{
		co = idx[ci];
		shares = (Rand(0, 30) + 70) / 100;
		k[1][co] = money * shares / comp_value[co];
		money -= k[1][co] * comp_value[co];
	}

	/* spend rest of money */
	co = idx[0]; /* cheapest share ??? */
	k[1][co] += money / comp_value[co];

	ClrMsg();
	ScrPos(X_START, MSG_LIN);
	PutStr("Aktien An/Verkauf: ");
	
	for (sb = 0; sb < 2; sb++)
	{
		for (co = 0; co < COMP_CNT; co++)
		{
			if (comp_value[co] == 0 || k[sb][co] == 0) continue;

			shares = k[sb][co];

			ScrPos(X_START, MSG_LIN + 1);
			ScrEmpf();
			PutStr(comp_name[co]);
			printf("? %ld", shares);
			ScrNorm();
			WaitSpace();
			play_shares[pl][co] += shares;
			/*comp_sold[co] = shares;*/
			play_money[pl] -= shares * comp_value[co];
			
			ShowSidebar(pl, co);
		}
	}
	
}

/****************************************************************************/
/* false = exit */

BOOL TradeComp(pl, co)
	int pl, co;
{
	long shares, money;
	
	ClrMsg();
	ScrPos(X_START, MSG_LIN);
	PutStr("Aktien An/Verkauf: ");
	
	while(TRUE)
	{
		ShowSidebar(pl, co);
		
		ScrPos(X_START + 19, MSG_LIN);
		ScrEmph();
		PutStr(comp_name[co]);
		ScrNorm();
		
		InputStr(X_START, MSG_LIN + 1, "?", buffer, 15);
		strtolower(buffer);

		shares = 0;
		money = 0;
	
		if (buffer[0] == '\0' || buffer[0] == '*')
		{
			/* skip ? */
			return TRUE;
		}
		else if (buffer[0] == 'e')
		{
			/* end */
			return FALSE;
		}
		else if (buffer[0] == 'r')
			/* remaining money */
			shares = play_money[pl] / comp_value[co];
		else if (strcmpi(buffer, currency) == 0)
		{
			/* currency n */
			/* remaining money */
			shares = play_money[pl] / comp_value[co];

			/*
			ScrPos(41,1);
			printf("TC1 pl=%d co=%d shares=%ld ", pl, co, shares);
			getchar();
			*/
		}
		else if (strncmpi(buffer, currency, 2) == 0)
		{
			money = atol(buffer + 3);

			/*
			ScrPos(41,2);
			printf("TC2 pl=%d co=%d money=%ld ", pl, co, money);
			getchar();
			*/

			if (money > 0) shares = money / comp_value[co];
		}
		else
		{
			shares = atol(buffer);

			/*
			ScrPos(41,3);
			printf("TC3 pl=%d co=%d shares=%ld ", pl, co, shares);
			getchar();
			*/
		
			if (play_shares[pl][co] + shares < 0) shares = -play_shares[pl][co]; /* sell all */

			if (shares * comp_value[co] > play_money[pl]) continue;
		}

		/*
		ScrPos(41,4);
		printf("TC4 pl=%d co=%d money=%ld shares=%ld ", pl, co, money, shares);
		getchar();
		*/

		play_shares[pl][co] += shares;
		/*comp_sold[co] += shares;*/
		play_money[pl] -= shares * comp_value[co];
		
		return;
	}
}

/****************************************************************************/

TradeShares(pl)
	int pl;
{
	int co, sum;
	BOOL exit;

	/*
	ScrPos(1,1);
	printf("TS1");
	getchar();
	*/

	sum = 0;
	for (co = 0; co < COMP_CNT; co++)
		sum += comp_value[co];
	if (sum == 0) return; /* no companies */

	if (play_isauto[pl])
	{
		TradeAuto(pl);
		return;
	}

	sprintf(buffer, "+n / -n / Rest / %-2s n / %-2s / ENTER = Naechste / E = Ende",
		currency, currency);
	BottomLine(buffer);

	exit = FALSE;
	while(!exit)
	{
		for (co = 0; co < COMP_CNT; co++)
		{
			/*
			ScrPos(41,2);
			printf("TS pl=%d co=%d", pl, co);
			getchar();
			*/
		
			if (comp_value[co] == 0) continue;
			
			if (!TradeComp(pl, co))
			{
				exit = TRUE;
				break;
			}
		}
	}
	
	ShowSidebar(pl, -1);
	ClrMsg();
}

/****************************************************************************/
/* one player move */

PlayPlayer(pl)
	int pl;
{
	int co;
	long value;

	sprintf(debbuf,"PlayPlayer %d: isauto=%d money=%ld\r\n",
		pl, play_isauto[pl], play_money[pl]);
	SI_PutStr(debbuf);

	for (co = 0; co<COMP_CNT; co++)
	{
		sprintf(debbuf," co=%d: shares=%ld\r\n",
			co, play_shares[pl][co]);
		SI_PutStr(debbuf);
	}
	
	ScrClr();
	ShowFrame(TRUE);
	ShowMap();
	ShowSidebar(pl, -1);
	PlayField(pl);

	sprintf(debbuf,"PlayPlayer %d: money=%ld\r\n",
		pl, play_money[pl]);
	SI_PutStr(debbuf);
	
	/* dividende */
	for (co = 0; co < COMP_CNT; co++)
	{
		value = play_shares[pl][co] * (long)comp_value[co];
		value = value / 100;
		play_money[pl] += value;
	}
	
	sprintf(debbuf,"PlayPlayer %d: money=%ld\r\n",
		pl, play_money[pl]);
	SI_PutStr(debbuf);

	/* trading */

	ShowFrame(FALSE);
	ShowSidebar(pl, -1);
	TradeShares(pl);
	
	SI_PutStr("----------");
}	

/****************************************************************************/

Bankrupt()
{
	int co, pl, x, y;
	
	while(TRUE)
	{
		co = Rand(0, COMP_CNT - 1);
		if (comp_value[co] != 0) break;
	}

	/* bankruptcy of company c */
	ClrMsg();

	ScrPos(X_START, MSG_LIN);
	ScrEmph();
	PutStr(announce);
	PutStr(comp_name[co]);
	ScrNorm();
	PutStr(" musste Konkurs anmelden.");
	ScrPos(X_START, MSG_LIN + 1);
	PutStr("10% des Aktienwertes werden ausgezahlt.");
	
	WaitSpace();

	/* remove all shares */
	for (pl = 0; pl < play_cnt; pl++)
	{
		/*
		ScrPos(41, 1 + pl);
		printf("Bank pl=%d mon=%ld", pl, play_money[pl]);
		*/

		play_money[pl] += (play_shares[pl][co] * comp_value[co] / 10);
		play_shares[pl][co] = 0;

		/*
		ScrPos(61, 1 + pl);
		printf("pl=%d mon=%ld", pl, play_money[pl]);
		getchar();
		*/
	}

	/* delete company */
	comp_values[co] = 0;
	/*comp_sold[co] = 0;*/

	/* remove company from map */
	for (y = 0; y < FLD_Y; y++)
		for (x = 0; x < FLD_X; x++)
			if (field[x][y] == FLD_COMP + co)
			{
				field[x][y] = FLD_FREE;
				ShowCell(x, y, FALSE);
			}
}

/****************************************************************************/

SharesFall(pl)
	int pl;
{
	int co, dec;
	
	for (co = 0; co < COMP_CNT; co++)
	{
		if (comp_value[co] <= 10 || Rand(0, 30) < 28) continue;
		
		dec = comp_value[co] * Rand(0, 80) / 100 + 10;
		comp_value[co] -= dec;
		
		ClrMsg();
		
		ScrPos(X_START, MSG_LIN);
		ScrEmph();
		PutStr(announce);
		ScrNorm();
		PutStr("Die Aktien von ");
		ScrEmph();
		PutStr(comp_name[co]);
		ScrNorm();
		PutStr(" sind um ");
		ScrEmph();
		printf("%d %s", dec, currency);
		ScrNorm();
		ScrPos(X_START, MSG_LIN + 1);
		PutStr("gefallen.");
		
		ShowSidebar(pl, -1);
		
		WaitSpace();
	}
}

/****************************************************************************/

PlayGame()
{
	int ro, pl, co, x, y,c;
	BOOL ok;

	/* init field */
	for(y=0; y<FLD_Y; y++)
		for(x=0; x<FLD_X; x++)
			field[x][y] = FLD_FREE;

#ifndef DEBUG
	/* set 5 stars */
	ok = GetMovePos(MOVE_CNT, move_x, move_y);
	for (c = 0; c < MOVE_CNT; c++)
	{
		/*printf("p=%d,%d\r\n", move_x[c], move_y[c]);*/
		field[move_x[c]][move_y[c]] = FLD_STAR;
	}
#endif	

	/* init companies */
	for(co = 0; co < COMP_CNT; co++)
	{
		comp_value[co] = 0;
		/*comp_sold[co] = 0L;*/
	}

	/* init player */
	for (pl=0; pl<play_cnt; pl++)
	{
		play_money[pl] = 6000L;
		for (co=0; co<COMP_CNT; co++)
			play_shares[pl][co] = 0L;
	}

#ifdef DEBUG
	SetGame();
#endif

	round = 0;
	while(TRUE)
	{
		round++;
		for (pl=0; pl<play_cnt; pl++)
		{
			PlayPlayer(pl);
			if (exit_game) return;
			
			/* create space through bankruptcy */
			if (GetFreeFields() < 20)
			{
				Bankrupt();
				ShowSidebar(pl, -1);
			}

			SharesFall(pl);
		}
	}
}

/****************************************************************************/

InputPlayers()
{
	int pl, co;
	
	ScrClr();
	MsgLine("S T A R  T R A D E R S", 1, TRUE);
	
	BottomLine(NULL);

	if (IsRunCpm())
	{
		while(TRUE)
		{
			InputStr(X_START, 3, "Seed:", buffer, 5);
			rand_seed = atoi(buffer);
			if (rand_seed > 0) break;
		}
	}
	else
		rand_seed = SeedRefresh();

	play_cnt = 2;
	strcpy(play_name[0], "Spieler AA");
	play_isauto[0] = TRUE;
	strcpy(play_name[1], "Spieler BB");
	play_isauto[1] = FALSE;
	
	return;
	

	while(TRUE)
	{
		InputStr(X_START, 5, "Anzahl Spieler (2-6):", buffer, 1);
		if (exit_game) return;
		play_cnt = atol(buffer);
		if (play_cnt >= 2 && play_cnt <= 6) break;
	}

	ScrPos(X_START, 7);
	PutStr("(Computern ein '*' voranstellen)");
	
	for (pl = 0; pl < play_cnt; pl++)
	{
		while(TRUE)
		{
			ScrPos(X_START, 9 + pl);
			printf("Spieler %d: ", pl+1);
			InputStr(0, 0, NULL, buffer, 9);
			if (strlen(buffer) > 0) break;
		}
		strcpy(play_name[pl], buffer);
	}
}

/****************************************************************************/

ShowHelp()
{
}

/****************************************************************************/

/* set game for debugging */

SetGame()
{
	field[1][4] = FLD_STAR;
	field[4][8] = FLD_STAR;
	field[6][3] = FLD_STAR;
	field[6][5] = FLD_STAR;
	field[9][7] = FLD_STAR;
	
	comp_value[0] = 1000;
	/*comp_sold[0] = 5L;*/
	comp_value[1] = 3000;
	/*comp_sold[1] = 5L;*/
	
	field[6][5] = FLD_COMP + 0;
	field[5][5] = FLD_COMP + 0;
	field[7][6] = FLD_COMP + 1;
	
	play_shares[0][0] = 5;
	play_shares[0][1] = 2;
	play_shares[1][0] = 2;
	play_shares[1][1] = 5;
}

BOOL SetMoves(cnt, xp, yp)
	int cnt;
	char xp[], yp[];
{
	xp[0] = 1;	yp[0] = 0;
	xp[1] = 6;	yp[1] = 2;
	xp[2] = 9;	yp[2] = 4;
	xp[3] = 6;	yp[3] = 6;
	xp[4] = 8;	yp[4] = 6;

}

/****************************************************************************/

main()
{
	int ch;
	int i;
	int r, max, rmax;

	exit_game = FALSE;
	ask_exit_active = FALSE;

	PutStr("\r\n--- startraders ---\r\n");

	/* SI_PutStr("\r\n\r\n--- startraders ---\r\n"); */
	PutStr("\r\n\r\n--- startraders ---\r\n");

	ScrClr();

	/* hier hilfe abfragen !!! */
	
	InputPlayers();

	PlayGame();

	ch = GetChr();

}

/****************************************************************************/
