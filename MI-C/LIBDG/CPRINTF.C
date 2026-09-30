/****************************************************************************/
/*
	CPRINTF.C  *dg*  09/2026 for MI-C compiler and CP/M

	PRINTF using CONIO
*/

extern putch();

/****************************************************************************/

cprintf(fmt, args)
	char *fmt;
	int *args;
{
	/* aufruf von print mit zeiger auf die parameterliste args
	   zeichenausgabe ueber CONIO funtion putch() */
	print(putch, fmt, &args);
}

/****************************************************************************/
