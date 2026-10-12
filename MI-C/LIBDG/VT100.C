/* VT100.C *dg* 12.10.2026 */
/* VT100 controls using CONIO */

#include "vt100.h"

unsigned graph = 0x00;
unsigned attr = 0x00;

/****************************************************************************/

int vt_attr()
{
	return attr | graph;
}

/****************************************************************************/
/* erase to end of line */

vt_eolera()
{
	cputs("\033[K");
}

/****************************************************************************/
/* erase to start of line */

vt_solera()
{
	cputs("\033[1K");
}

/****************************************************************************/
/* erase line */

vt_linera()
{
	cputs("\033[2K");
}

/****************************************************************************/
/* erase screen from current line down to end */

vt_dwnera()
{
	cputs("\033[J");
}

/****************************************************************************/
/* erase screen from current line up to top */

vt_upera()
{
	cputs("\033[1J");
}

/****************************************************************************/
/* erase entire screen and home */

vt_screra()
{
	cputs("\033[2J");
	cputs("\033[H");
}

/****************************************************************************/
/* cursor home, pos 1,1 */

vt_home()
{
	cputs("\033[H");
}

/****************************************************************************/
/* cursor position, x = 1..80 / y = 1..25
*/

vt_pos(x,y)
	int x,y;
{
	char s[3];
	
	if (x < 1 || x > 80) return;
	if (y < 1 || y > 25) return;
	
	cputs("\33{");
	itoa(y, s);
	cputs(s);
	putch(';');
	itoa(x, s);
	cputs(s);
	putch('H');
}

/****************************************************************************/
/* reset all attributs */

vt_norm()
{
	cputs("\033[0m");
	attr = 0x00;
}

/****************************************************************************/
/* emphasized / bright */

vt_emph()
{
	cputs("\033[1m");
	attr |= VT_EMPH;
}

/****************************************************************************/

/*
vt_dim()
{
	cputs("\033[2m");
	attr |= VT_DIM;
}
*/

/****************************************************************************/
/* underscore */

vt_under()
{
	cputs("\033[4m");
	attr |= VT_UNDER;
}

/****************************************************************************/
/* blink */

vt_blink()
{
	cputs("\033[5m");
	attr |= VT_BLINK;
}

/****************************************************************************/
/* invers */

vt_inv()
{
	cputs("\033[7m");
	attr |= VT_INV;
}

/****************************************************************************/
/* hidden */

/*
vt_hidden()
{
	cputs("\033[8m");
	attr |= VT_HIDDEN;
}
*/

/****************************************************************************/

vt_forecol(col)
	int col;
{
	char s[3];
	
	cputs("\033[");
	itoa(col + 30, s);
	cputs(s);
	putch('m');
}

/****************************************************************************/

vt_backcol(col)
	int col;
{
	char s[3];
	
	cputs("\033[");
	itoa(col + 40, s);
	cputs(s);
	putch('m');
}

/****************************************************************************/
/* switch to special graphics characters set and line drawing set */

vt_gon()
{
	cputs("\033(0");
	graph = VT_GRAPH;
}

/****************************************************************************/
/* switch to US ASCII */

vt_goff()
{
	cputs("\033(B");
	graph = 0x00;
}

/****************************************************************************/
/* output string with control codes

    hex  oct
	0x01 001 = norm
	0x02 002 = dim
	0x03 003 = emph
	0x04 004 = underscore
	0x05 005 = blink
	0x06 006 = hidden
	0x07 007 = invers
	0x05 010 = graph on
	0x06 011 = graph off
*/

vt_puts(s)
	char *s;
{
	while(*s)
	{
		switch(*s)
		{
			case 001:
				vt_norm();
				break;
			/*
			case 002:
				vt_dim();
				break;
			*/
			case 003:
				vt_emph();
				break;
			case 004:
				vt_underscore();
				break;
			case 005:
				vt_blink();
				break;
			/*
			case 006:
				vt_hidden();
				break;
			*/
			case 007:
				vt_inv();
				break;
			case 010:
				vt_gon();
				break;
			case 011:
				vt_goff();
				break;
			default:
				putch(*s);
		}
		s++;
	}
	/*ScrNorm();*/
}

/****************************************************************************/

/* end of file */
