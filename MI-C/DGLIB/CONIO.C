/* conio.h combines all conio funtions ny using
   nonblocking BDOS functions */

/* make them local */
static int lastch = -1;


/* putch() using BDOS function 6 */
int putch(c)
	int c;
{
	BDOS(c, 6);
	return c;
}


/* getch() using nonblocking BDOS function 6 */
int getch()
{
	register int c;
	if (lastch != -1)
	{
		c = lastch;
		lastch = -1;
		return c;
	}
	
	while(1)
	{
		c = BDOS(0xFF, 6);
		if (c != 0) return c;
	}
}


/* getch() using nonblocking BDOS function 6 */
/* echos character to console */
int getche()
{
	register int c;
	c = getch();
	putch(c);
	return c;
}


/* return 1 if character from console is available and 0 if not */
int kbhit()
{
	if (lastch != -1) return 1;
	return BDOS(0, 11) & 0x01;
}


/* puts one character back into the input buffer */
int ungetch(c)
	int c;
{
	if (c != -1) lastch = c;
}


/* outputs a string to console */
cputs(ptr)
	char *ptr;
{
	while(*ptr)
		putch(*ptr++);
}


/* end of file */
