/* conio.h combines all conio funtions by using
   nonblocking BDOS functions */

/* static makes them local */
static int lastch = -1;
static int hitch = -1;


/* putch() using BDOS function 6 */
int putch(c)
	int c;
{
	if (c != 0xFF)
	{
		BDOS(c, 6);
	}
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
	if (hitch != -1)
	{
		c = hitch;
		hitch = -1;
		return c;
	}
	
	while(1)
	{
		c = BDOS(0xFF, 6);
		if (c != 0) return c;
	}
}


/* getche() using nonblocking BDOS function 6 */
/* echos character to console */
int getche()
{
	register int c;
	c = getch();
	putch(c);
	return c;
}


/* kbhit() using nonblocking BDOS function 6 */
/* return 1 if character from console is available and 0 if not */
int kbhit()
{
	int c;
	
	if (hitch != -1 || lastch != -1) return 1;

	c = BDOS(0xFF, 6);
	if (c == 0) return 0;

	hitch = c;
	return 1;
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
